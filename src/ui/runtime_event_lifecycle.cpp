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
  current_event_route_ = EventRouter::route_to_root(event, root_view_id_);
  last_event_result_ = EventResult::unhandled();
  last_event_dispatch_ = EventDispatchRecord{
      .sequence = ++event_dispatch_sequence_,
      .view_id = current_event_route_->target_view_id,
      .event_kind = current_event_route_->event_kind,
      .route = *current_event_route_,
      .result = last_event_result_};
  if (after_event_callback_) {
    after_event_callback_(context(), *last_event_dispatch_);
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
  current_event_route_ = EventRouter::route_to_root(event, record.root_view_id);
  last_event_result_ = EventResult::unhandled();
  last_event_dispatch_ = EventDispatchRecord{
      .sequence = ++event_dispatch_sequence_,
      .view_id = current_event_route_->target_view_id,
      .event_kind = current_event_route_->event_kind,
      .route = *current_event_route_,
      .result = last_event_result_};
  if (after_event_callback_) {
    after_event_callback_(context_for_record(record), *last_event_dispatch_);
  }
  drain_deferred_callbacks();
  flush_deferred_redraw_request();
}

} // namespace cgpui
