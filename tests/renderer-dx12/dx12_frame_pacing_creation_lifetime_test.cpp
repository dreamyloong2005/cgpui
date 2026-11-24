#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  Dx12PacingForwardApi api; // This reference owner outlives both renderer and orphan frame.
  if (!deadline.done || !window.hwnd || !pacing_begin_debug_enable())
    return dx12_fail(1, "pacing creation lifetime native setup failed");
  auto created = dx12_create_renderer(window.descriptor(), api);
  if (!created || api.sets != 1 || api.gets != 1 || api.closes)
    return dx12_fail(2, "private seam did not create a real configured renderer");
  auto renderer = std::move(*created);
  const auto before = renderer->backend_info();
  if (!dx12_remove_native_device(before)) return dx12_fail(3, "real pre-frame device removal failed");
  const auto retry = renderer->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required ||
      api.closes != 1 || !api.close_ok || api.sets != 2 || api.gets != 2 ||
      renderer->backend_info().adapter_luid != before.adapter_luid ||
      renderer->backend_info().device_generation != before.device_generation + 1 ||
      !pacing_begin_debug_clean(renderer->backend_info()))
    return dx12_fail(4, "production recovery lost its borrowed API, old close or new acquisition");
  auto orphan = pacing_begin_bounded(*renderer, api);
  if (!orphan || !*orphan) return dx12_fail(5, "real post-recovery frame unavailable");
  const auto after = renderer->backend_info();
  const auto polls = api.polls;
  renderer.reset();
  if (api.closes != 1) return dx12_fail(6, "renderer destruction closed state retained by an orphan frame");
  const auto capture = (*orphan)->request_pixel_capture();
  const auto presented = (*orphan)->present();
  if (!capture || !presented || (*orphan)->present() || api.closes != 1 ||
      api.sets != 2 || api.gets != 2 || api.polls != polls || !pacing_begin_debug_clean(after))
    return dx12_fail(7, "healthy recovered orphan must finish once without new pacing/native ownership");
  orphan->reset();
  return api.closes == 2 && api.close_ok
      ? 0 : dx12_fail(8, "last frame release did not close the replacement latency handle once");
}
