#include "dx12_pacing_control_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  Dx12PacingControlApi api;
  if (!deadline.done || !window.hwnd || !pacing_begin_debug_enable()) return dx12_fail(1, "setup failed");
  auto created = dx12_create_renderer(window.descriptor(), api);
  if (!created) return dx12_fail(2, created.error().message.c_str());
  auto renderer = std::move(*created);
  const auto initial = renderer->backend_info();
  auto pending = renderer->begin_frame();
  if (pending || pending.error().code != ErrorCode::frame_not_ready || api.polls != 1 ||
      renderer->last_frame_pixels() || renderer->last_frame_diagnostic_snapshot() ||
      renderer->backend_info().device_generation != initial.device_generation)
    return dx12_fail(3, "scripted timeout did not leave initial real renderer state unchanged");
  api.waited = {WAIT_OBJECT_0, ERROR_SUCCESS};
  auto abandoned = renderer->begin_frame();
  if (!abandoned || !*abandoned || api.polls != 2) return dx12_fail(4, "ready did not admit");
  auto overlap = renderer->begin_frame();
  if (overlap || overlap.error().code != ErrorCode::invalid_argument || api.polls != 2)
    return dx12_fail(5, "overlap must fail before polling");
  abandoned->reset();
  api.waited = {WAIT_TIMEOUT, ERROR_SUCCESS};
  if (!renderer->resize({96, 96}, {1}) || api.forwarded.gets != 1)
    return dx12_fail(6, "same-chain resize replaced admission ownership");
  auto invalid = renderer->begin_frame();
  if (!invalid || api.polls != 2) return dx12_fail(7, "abandon/resize consumed ready grant");
  (*invalid)->clear({std::numeric_limits<float>::quiet_NaN(), 0, 0, 1});
  auto rejected = (*invalid)->present();
  if (rejected || rejected.error().code != ErrorCode::invalid_argument)
    return dx12_fail(8, "pre-Present invalid draw accepted");
  invalid->reset();
  auto first = renderer->begin_frame();
  if (!first || api.polls != 2 || !pacing_control_capture(**first, *renderer))
    return dx12_fail(9, "pre-Present rejection consumed grant or native capture failed");
  first->reset();
  const auto* pixels = renderer->last_frame_pixels();
  const auto* snapshot = renderer->last_frame_diagnostic_snapshot();
  if (pixels->width != 96 || pixels->height != 96)
    return dx12_fail(13, "production ResizeBuffers did not change actual capture dimensions");
  const auto rgba = pixels->rgba8;
  for (unsigned i = 0; i != 3; ++i) {
    auto next = renderer->begin_frame();
    if (next || next.error().code != ErrorCode::frame_not_ready || api.polls != 3 + i ||
        renderer->last_frame_pixels() != pixels || pixels->rgba8 != rgba ||
        renderer->last_frame_diagnostic_snapshot() != snapshot ||
        snapshot->submitted_draws.solid_rect_count != 1 ||
        renderer->backend_info().device_generation != initial.device_generation)
      return dx12_fail(10, "post-Present timeout consumed capture/snapshot or retained old grant");
  }
  api.waited = {WAIT_OBJECT_0, ERROR_SUCCESS};
  auto resumed = renderer->begin_frame();
  if (!resumed || !pacing_control_capture(**resumed, *renderer) ||
      !pacing_begin_debug_clean(renderer->backend_info()))
    return dx12_fail(11, "pending poisoned real renderer or debug state");
  resumed->reset();
  renderer.reset();
  if (api.forwarded.closes != 1 || !api.forwarded.close_ok) return dx12_fail(12, "handle lifetime failed");
  std::cout << "PACING_CONTROL scripted-wait native-renderer pending/abandon/overlap/capture PASS\n";
  return 0;
}
