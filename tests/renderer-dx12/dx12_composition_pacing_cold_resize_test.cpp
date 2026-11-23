#include "dx12_composition_first_begin_test_support.hpp"
#include "dx12_composition_pacing_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_popup_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready() || !pacing_begin_debug_enable())
    return dx12_fail(1, "cold composition resize native setup unavailable");
  Dx12CompositionFirstBeginWindows windows;
  Dx12PacingForwardApi api;
  if (!windows.matches(false)) return dx12_fail(2, "initially-hidden composition fixture differs");
  auto created = dx12_create_renderer(windows.descriptor(), api);
  if (!created) return dx12_fail(3, created.error().message.c_str());
  auto renderer = std::move(*created);
  const auto info = renderer->backend_info();
  const auto handle = api.observed_handle;
  if (!handle || api.sets != 1 || api.gets != 1 || api.polls || api.ready || api.closes ||
      renderer->surface_composition().composite_alpha != RendererCompositeAlpha::pre_multiplied)
    return dx12_fail(4, "cold chain already polled or lacked configured composition admission");
  // Before ANY begin_frame/Present/Commit, change both the real HWND and chain extent.
  if (!composition_pacing_resize_hidden_window(windows, 96) || !renderer->resize({96, 96}, {1}) ||
      api.polls || api.ready || api.sets != 1 || api.gets != 1 || api.closes ||
      api.observed_handle != handle)
    return dx12_fail(5, "cold resize polled admission or changed the owned handle epoch");
  auto first = pacing_begin_bounded(*renderer, api);
  if (!first || !*first || !api.polls || api.ready != 1 ||
      !composition_pacing_capture(**first, *renderer, false, 96))
    return dx12_fail(6, "resized cold chain did not acquire native readiness and real96 pixels");
  first->reset();
  const auto polls = api.polls;
  // A second real resize after Present must not manufacture a new cached grant.
  if (!composition_pacing_resize_hidden_window(windows, 64) || !renderer->resize({64, 64}, {1}) ||
      api.polls != polls || api.ready != 1 || api.sets != 1 || api.gets != 1 ||
      api.closes || api.observed_handle != handle)
    return dx12_fail(7, "post-Present resize reacquired or prematurely consumed readiness");
  auto second = pacing_begin_bounded(*renderer, api);
  if (!second || !*second || api.polls <= polls || api.ready != 2 ||
      !composition_pacing_capture(**second, *renderer, true, 64) || !windows.matches(false) ||
      renderer->backend_info().adapter_luid != info.adapter_luid ||
      renderer->backend_info().device_generation != info.device_generation ||
      !pacing_begin_debug_clean(renderer->backend_info()))
    return dx12_fail(8, "post-Present resized frame lacks fresh readiness/pixels/debug proof");
  second->reset();
  renderer.reset();
  composition_pacing_trace("cold-and-post-present-resize-closed", api, info);
  return api.sets == 1 && api.gets == 1 && api.closes == 1 && api.close_ok
      ? 0 : dx12_fail(9, "both real resizes did not retain exactly one owned handle");
}
