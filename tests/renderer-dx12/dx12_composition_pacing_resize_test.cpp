#include "dx12_composition_first_begin_test_support.hpp"
#include "dx12_composition_pacing_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_popup_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready() || !pacing_begin_debug_enable())
    return dx12_fail(1, "composition pacing resize native setup unavailable");
  Dx12CompositionFirstBeginWindows windows; // Reuse A's never-shown construction.
  Dx12PacingForwardApi api; // Outlives renderer and every retained frame.
  if (!windows.matches(false)) return dx12_fail(2, "initially-hidden composition fixture differs");
  auto created = dx12_create_renderer(windows.descriptor(), api);
  if (!created) return dx12_fail(3, created.error().message.c_str());
  auto renderer = std::move(*created);
  const auto info = renderer->backend_info();
  if (!info.adapter_luid || !info.device_generation || info.backend != RendererBackend::dx12 ||
      info.selection != RendererBackendSelection::dx12 || !info.fallback_reason.empty() ||
      renderer->surface_composition().composite_alpha != RendererCompositeAlpha::pre_multiplied ||
      api.sets != 1 || api.gets != 1 || !api.observed_handle || api.polls || api.closes)
    return dx12_fail(4, "real composition pacing owner not configured exactly once");
  auto abandoned = pacing_begin_bounded(*renderer, api);
  if (!abandoned || !*abandoned || api.ready != 1 || api.polls == 0)
    return dx12_fail(5, "first real composition frame lacked strictly forwarded native readiness");
  const auto polls = api.polls;
  const auto handle = api.observed_handle; // Compare only within this same-chain operation.
  const auto overlap = renderer->begin_frame(); // Negative oracle is raw, never retried.
  if (overlap || overlap.error().code != ErrorCode::invalid_argument || api.polls != polls)
    return dx12_fail(6, "active overlap reached pacing or was accepted");
  abandoned->reset(); // CPU abandonment must retain the already observed grant.
  if (!composition_pacing_resize_hidden_window(windows, 96) || !renderer->resize({96, 96}, {1}) ||
      api.sets != 1 || api.gets != 1 || api.closes || api.observed_handle != handle)
    return dx12_fail(7, "real 64-to-96 resize changed handle/configuration or showed HWND");
  auto invalid = pacing_begin_bounded(*renderer, api);
  if (!invalid || !*invalid || api.polls != polls || api.ready != 1)
    return dx12_fail(8, "abandon/resize consumed the cached native grant");
  (*invalid)->clear({std::numeric_limits<float>::quiet_NaN(), 0, 0, 1});
  const auto rejected = (*invalid)->present(); // Pre-native-Present rejection stays raw.
  if (rejected || rejected.error().code != ErrorCode::invalid_argument || api.polls != polls)
    return dx12_fail(9, "pre-Present validation did not reject without polling");
  invalid->reset();
  auto first = pacing_begin_bounded(*renderer, api);
  if (!first || !*first || api.polls != polls || api.ready != 1 ||
      !composition_pacing_capture(**first, *renderer, false, 96))
    return dx12_fail(10, "validation consumed cached grant or actual resized premultiplied capture failed");
  first->reset();
  const auto after_present_polls = api.polls;
  auto second = pacing_begin_bounded(*renderer, api);
  if (!second || !*second || api.polls <= after_present_polls || api.ready != 2 ||
      !composition_pacing_capture(**second, *renderer, true, 96))
    return dx12_fail(11, "post-Present frame reused stale admission or stale red pixels");
  second->reset();
  if (!composition_pacing_hidden_extent(windows, 96) ||
      renderer->backend_info().adapter_luid != info.adapter_luid ||
      renderer->backend_info().device_generation != info.device_generation ||
      !pacing_begin_debug_clean(renderer->backend_info()))
    return dx12_fail(12, "resize/native presentation changed topology, epoch or debug safety");
  composition_pacing_trace("resize-before-close", api, renderer->backend_info());
  renderer.reset();
  composition_pacing_trace("resize-after-close", api, info);
  return api.sets == 1 && api.gets == 1 && api.closes == 1 && api.close_ok
      ? 0 : dx12_fail(13, "same composition chain did not close exactly once");
}
