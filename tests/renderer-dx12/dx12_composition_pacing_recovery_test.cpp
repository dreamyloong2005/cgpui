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
    return dx12_fail(1, "composition pacing recovery native setup unavailable");
  Dx12CompositionFirstBeginWindows windows;
  Dx12PacingForwardApi api; // Borrowed by old and replacement epochs until all frames die.
  if (!windows.matches(false)) return dx12_fail(2, "initially-hidden composition fixture differs");
  auto created = dx12_create_renderer(windows.descriptor(), api);
  if (!created) return dx12_fail(3, created.error().message.c_str());
  auto renderer = std::move(*created);
  const auto before = renderer->backend_info();
  if (!before.adapter_luid || !before.device_generation || before.backend != RendererBackend::dx12 ||
      before.selection != RendererBackendSelection::dx12 || !before.fallback_reason.empty() ||
      renderer->surface_composition().composite_alpha != RendererCompositeAlpha::pre_multiplied ||
      api.sets != 1 || api.gets != 1 || !api.observed_handle || api.polls || api.closes)
    return dx12_fail(4, "real composition pacing epoch unavailable");
  auto finished_old = pacing_begin_bounded(*renderer, api);
  if (!finished_old || !*finished_old || api.ready != 1 ||
      !composition_pacing_capture(**finished_old, *renderer, false, 64))
    return dx12_fail(5, "pre-loss actual first admission/premultiplied red pixels failed");
  auto granted_old = pacing_begin_bounded(*renderer, api);
  if (!granted_old || !*granted_old || api.ready != 2)
    return dx12_fail(6, "pre-loss next native admission unavailable");
  granted_old->reset(); // Keep a cached, unconsumed grant in the epoch about to be removed.
  const auto old_polls = api.polls;
  const auto old_ready = api.ready;
  if (!dx12_remove_native_device(before))
    return dx12_fail(7, "actual native RemoveDevice failed");
  const auto retry = renderer->begin_frame(); // Exactly one RAW typed recovery retry oracle.
  const auto after = renderer->backend_info();
  if (retry || retry.error().code != ErrorCode::frame_retry_required ||
      api.closes != 1 || !api.close_ok || api.sets != 2 || api.gets != 2 ||
      api.polls != old_polls || api.ready != old_ready ||
      after.adapter_luid != before.adapter_luid || after.software_adapter != before.software_adapter ||
      after.device_generation != before.device_generation + 1 || renderer->last_frame_pixels() ||
      !windows.matches(false) || !pacing_begin_debug_clean(after))
    return dx12_fail(8, "recovery failed old close/new configure, typed retry, same-LUID generation or hidden state");
  composition_pacing_trace("recovery-after-typed-retry", api, after);
  auto replacement = pacing_begin_bounded(*renderer, api); // Success only; typed retry is never swallowed.
  if (!replacement || !*replacement || api.polls <= old_polls || api.ready != old_ready + 1)
    return dx12_fail(9, "replacement chain reused old cached grant instead of native readiness");
  const auto replacement_polls = api.polls;
  const auto duplicate = (*finished_old)->present(); // Old finished frame cannot touch the new grant.
  if (duplicate || duplicate.error().code != ErrorCode::invalid_argument || api.polls != replacement_polls)
    return dx12_fail(10, "old finished frame touched replacement admission");
  finished_old->reset();
  const auto overlap = renderer->begin_frame(); // Old destruction must not release the active new frame.
  if (overlap || overlap.error().code != ErrorCode::invalid_argument || api.polls != replacement_polls)
    return dx12_fail(11, "old frame destruction unlocked the active replacement");
  if (!composition_pacing_capture(**replacement, *renderer, true, 64) ||
      !windows.matches(false) || !pacing_begin_debug_clean(after) ||
      !dx12_recovered_native_device_clean(after))
    return dx12_fail(12, "replacement native device failed fresh blue premultiplied pixels/debug");
  replacement->reset();
  renderer.reset();
  composition_pacing_trace("recovery-after-close", api, after);
  return api.sets == 2 && api.gets == 2 && api.closes == 2 && api.close_ok
      ? 0 : dx12_fail(13, "old and replacement composition epochs did not each close once");
}
