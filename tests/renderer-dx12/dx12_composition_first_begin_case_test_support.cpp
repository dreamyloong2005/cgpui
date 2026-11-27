#include "dx12_composition_first_begin_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_popup_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"
#include <cstdlib>
#include <string_view>

namespace cgpui_test {
int composition_first_begin_case(bool visible) {
  using namespace cgpui;
  Dx12RecoveryDeadline deadline{visible ? DesktopTestAccess::exclusive : DesktopTestAccess::none};
  if (!deadline.done || !dx12_popup_desktop_ready() || !pacing_begin_debug_enable())
    return dx12_fail(1, "composition first-begin desktop/debug/watchdog unavailable");
  Dx12CompositionFirstBeginWindows windows;
  if (!windows.matches(false)) return dx12_fail(2, "real initially-hidden composition topology unavailable");
  if (visible && !windows.show_explicitly())
    return dx12_fail(3, "explicit visible composition condition unavailable");
  if (!windows.matches(visible)) return dx12_fail(4, "native condition changed before factory");
  Dx12PacingForwardApi api; // Declared before every renderer/frame sharing this borrowed API.
  const auto trace = [&](const char* stage) {
    std::cout << "COMPOSITION_FIRST_BEGIN stage=" << stage
              << " condition=" << (visible ? "explicitly-visible" : "initially-hidden")
              << " sets=" << api.sets << " gets=" << api.gets << " polls=" << api.polls
              << " ready=" << api.ready << " pending=" << api.pending << " closes=" << api.closes
              << " native_condition=" << windows.matches(visible) << std::endl;
  };
  auto created = dx12_create_renderer(windows.descriptor(), api);
  if (!created) { trace("factory-failed"); return dx12_fail(5, created.error().message.c_str()); }
  auto renderer = std::move(*created);
  const auto info = renderer->backend_info();
  const char* requested = std::getenv("CGPUI_DX12_ADAPTER");
  const bool warp = requested && std::string_view(requested) == "warp";
  if (info.backend != RendererBackend::dx12 || info.selection != RendererBackendSelection::dx12 ||
      !info.adapter_luid || !info.device_generation || !info.fallback_reason.empty() ||
      (warp && !info.software_adapter) || !renderer->popup_windows_supported() ||
      renderer->surface_composition().composite_alpha != RendererCompositeAlpha::pre_multiplied ||
      !pacing_begin_debug_clean(info))
    return dx12_fail(6, "actual DX12 composition renderer identity/debug state differs");
  std::cout << "COMPOSITION_FIRST_BEGIN adapter=" << info.adapter_name
            << " luid=" << *info.adapter_luid << " software=" << info.software_adapter
            << " generation=" << info.device_generation << std::endl;
  trace("before-first-begin"); // Zero setup counters are evidence, NOT an early failure.
  if (!windows.matches(visible)) return dx12_fail(7, "native condition changed before first begin");
  // Existing helper calls the real virtual begin_frame and only retries genuine pending.
  // Its finite pump/yield runs after begin returns; it never waits on the handle separately.
  auto frame = pacing_begin_bounded(*renderer, api);
  trace(frame && *frame ? "returned-frame" : "no-frame");
  if (!frame || !*frame)
    return dx12_fail(8, frame ? "null first frame" : frame.error().message.c_str());
  int verdict = 0;
  if (!windows.matches(visible)) verdict = dx12_fail(9, "native condition changed during first begin");
  else if (api.polls == 0 || api.ready != 1)
    verdict = dx12_fail(40, "FAIL_UNPACED_FIRST_FRAME: real composition begin_frame returned before native ready");
  else if (api.sets != 1 || api.gets != 1 || !api.observed_handle || api.closes || !api.close_ok)
    verdict = dx12_fail(10, "admitted first frame did not belong to one owned native pacing epoch");
  else if (renderer->backend_info().device_generation != info.device_generation ||
           !pacing_begin_debug_clean(renderer->backend_info()))
    verdict = dx12_fail(11, "first admission changed generation or produced debug errors");
  // This A oracle does not draw, submit, Present, or precommit a graph to prime readiness.
  // Destroy the never-presented CPU frame before the facade, and both before the API/HWNDs.
  frame->reset();
  renderer.reset();
  trace("after-cleanup");
  const unsigned expected_closes = api.observed_handle ? 1U : 0U;
  if (api.closes != expected_closes || !api.close_ok)
    return dx12_fail(12, "first-begin cleanup did not close exactly its acquired native handle");
  return verdict;
}
} // namespace cgpui_test
