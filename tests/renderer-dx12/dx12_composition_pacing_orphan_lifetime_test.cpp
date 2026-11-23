#include "dx12_composition_first_begin_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_popup_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
int orphan_case(bool finished_first) {
  Dx12CompositionFirstBeginWindows windows; // Genuinely never shown in either pass.
  Dx12PacingForwardApi api; // Must outlive the facade and every frame's shared state.
  if (!windows.matches(false)) return dx12_fail(2, "hidden composition lifetime HWND unavailable");
  auto created = dx12_create_renderer(windows.descriptor(), api);
  if (!created) return dx12_fail(3, created.error().message.c_str());
  auto renderer = std::move(*created);
  const auto info = renderer->backend_info();
  if (info.backend != RendererBackend::dx12 || info.selection != RendererBackendSelection::dx12 ||
      !info.adapter_luid || !info.device_generation || !info.fallback_reason.empty() ||
      renderer->surface_composition().composite_alpha != RendererCompositeAlpha::pre_multiplied ||
      api.sets != 1 || api.gets != 1 || !api.observed_handle || api.polls || api.closes ||
      !pacing_begin_debug_clean(info))
    return dx12_fail(4, "composition lifetime did not create one real native pacing epoch");
  auto finished = pacing_begin_bounded(*renderer, api); // Only successful acquisitions use the helper.
  if (!finished || !*finished || api.ready != 1 || api.polls == 0)
    return dx12_fail(5, "composition lifetime first frame lacked native readiness");
  (*finished)->clear({1, 0, 0, 0.5F});
  if (!(*finished)->request_pixel_capture() || !(*finished)->present() ||
      !dx12_pixel(renderer->last_frame_pixels(), 32, 32, {128, 0, 0, 128}, 2) ||
      !windows.matches(false) || !pacing_begin_debug_clean(renderer->backend_info()))
    return dx12_fail(6, "finished composition frame lacks actual premultiplied capture/debug proof");
  const auto first_polls = api.polls;
  const auto repeated_live = (*finished)->present(); // Raw negative call; never retry an error oracle.
  if (repeated_live || repeated_live.error().code != ErrorCode::invalid_argument ||
      api.polls != first_polls || api.ready != 1 || api.closes)
    return dx12_fail(7, "already-finished frame was accepted or touched native pacing");
  auto orphan = pacing_begin_bounded(*renderer, api);
  if (!orphan || !*orphan || api.ready != 2 || api.polls <= first_polls)
    return dx12_fail(8, "active composition orphan lacked fresh post-Present native admission");
  const auto polls = api.polls;
  const auto overlap = renderer->begin_frame(); // Raw overlap assertion while the admitted frame is active.
  if (overlap || overlap.error().code != ErrorCode::invalid_argument || api.polls != polls)
    return dx12_fail(9, "overlap bypassed the active-frame contract or consumed readiness");
  renderer.reset(); // A finished and an active frame now both outlive the facade.
  if (api.closes) return dx12_fail(10, "facade destruction closed state retained by orphan frames");
  const auto repeated_orphan = (*finished)->present();
  const auto finished_capture = (*finished)->request_pixel_capture();
  if (repeated_orphan || repeated_orphan.error().code != ErrorCode::invalid_argument ||
      finished_capture || finished_capture.error().code != ErrorCode::invalid_argument)
    return dx12_fail(11, "finished ownerless frame regained presentation/capture permission");
  if (finished_first) {
    finished->reset(); // The still-active orphan alone must retain the state's native lifetime.
    if (api.closes) return dx12_fail(12, "finished-frame destruction closed the surviving active orphan");
  }
  (*orphan)->clear({0, 0, 1, 0.5F}); // CPU recording does not restore its destroyed owner.
  const auto dead_capture = (*orphan)->request_pixel_capture();
  const auto dead_present = (*orphan)->present();
  const auto repeated_dead = (*orphan)->present();
  if (dead_capture || dead_capture.error().code != ErrorCode::invalid_argument ||
      dead_present || dead_present.error().code != ErrorCode::invalid_argument ||
      repeated_dead || repeated_dead.error().code != ErrorCode::invalid_argument ||
      api.sets != 1 || api.gets != 1 || api.polls != polls || api.ready != 2 || api.closes ||
      !windows.matches(false) || !pacing_begin_debug_clean(info))
    return dx12_fail(13, "invalid-owner/finished rejection changed native pacing or hidden lifetime");
  orphan->reset();
  if (!finished_first && api.closes)
    return dx12_fail(14, "active-frame destruction closed state retained by the finished frame");
  finished->reset(); // Whichever frame was last performs actual state/native retirement.
  if (api.closes != 1 || !api.close_ok || api.sets != 1 || api.gets != 1 ||
      api.polls != polls || !windows.matches(false))
    return dx12_fail(15, "last-frame destruction did not close exactly one app-owned native handle");
  orphan->reset(); finished->reset(); renderer.reset();
  if (api.closes != 1) return dx12_fail(16, "empty owner cleanup closed the native handle twice");
  std::cout << "COMPOSITION_ORPHAN_LIFETIME finished_first=" << finished_first
            << " adapter=" << info.adapter_name << " luid=" << *info.adapter_luid
            << " software=" << info.software_adapter << " generation=" << info.device_generation
            << " sets=" << api.sets << " gets=" << api.gets << " ready=" << api.ready
            << " polls=" << api.polls << " closes=" << api.closes << std::endl;
  return 0;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready() || !pacing_begin_debug_enable())
    return dx12_fail(1, "composition lifetime desktop/debug/watchdog unavailable");
  for (const bool finished_first : {false, true})
    if (const int result = orphan_case(finished_first)) return result;
  return 0;
}
