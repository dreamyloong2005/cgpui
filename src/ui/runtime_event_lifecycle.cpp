#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::record_lifecycle_event(const PlatformEvent& event) {
  sync_root_input_record();
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::window_lifecycle,
      .event_kind = event_kind_for(event),
      .backend = "runtime",
      .operation = "window-lifecycle",
      .supported = true,
      .succeeded = true,
      .value_count = 1,
  });
  state_->input.current_event_route_ = EventRouter::route_to_root(event, state_->views.root_view_id_);
  state_->input.last_event_result_ = EventResult::unhandled();
  state_->diagnostics.last_event_dispatch_ = EventDispatchRecord{
      .sequence = ++state_->diagnostics.event_dispatch_sequence_,
      .view_id = state_->input.current_event_route_->target_view_id,
      .event_kind = state_->input.current_event_route_->event_kind,
      .route = *state_->input.current_event_route_,
      .result = state_->input.last_event_result_};
  sync_root_event_record();
  if (state_->core.after_event_callback_) {
    state_->core.after_event_callback_(context(), *state_->diagnostics.last_event_dispatch_);
  }
  drain_deferred_callbacks();
  flush_deferred_redraw_request();
}

void WindowRuntime::record_lifecycle_event_for_record(
    WindowRuntimeRecord& record,
    const PlatformEvent& event) {
  if (record.window == nullptr || record.renderer == nullptr) {
    return;
  }

  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::window_lifecycle,
      .event_kind = event_kind_for(event),
      .backend = "runtime",
      .operation = "window-lifecycle",
      .supported = true,
      .succeeded = true,
      .value_count = 1,
  });
  begin_event_route_for_record(record, event);
  finish_event_dispatch_for_record(record, EventResult::unhandled());
}

} // namespace cgpui
