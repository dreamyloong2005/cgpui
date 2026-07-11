#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::begin_event_route_for_record(
    WindowRuntimeRecord& record,
    const PlatformEvent& event) {
  record.event_route = EventRouter::route_to_root(event, record.root_view_id);
  refresh_route_ancestry(*record.event_route);
}

void WindowRuntime::finish_event_dispatch_for_record(
    WindowRuntimeRecord& record,
    EventResult result) {
  if (!record.event_route.has_value()) {
    return;
  }
  record.last_event_result = result;
  record.last_event_dispatch = EventDispatchRecord{
      .sequence = ++event_dispatch_sequence_,
      .view_id = record.event_route->target_view_id,
      .event_kind = record.event_route->event_kind,
      .route = *record.event_route,
      .result = record.last_event_result};
  if (after_event_callback_) {
    after_event_callback_(context_for_record(record), *record.last_event_dispatch);
  }
  drain_deferred_callbacks();
  flush_deferred_redraw_request();
}

void WindowRuntime::sync_root_event_record() {
  WindowRuntimeRecord* record =
      find_window_runtime_record(root_window_runtime_id_);
  if (record != nullptr) {
    record->event_route = current_event_route_;
    record->last_event_result = last_event_result_;
    record->last_event_dispatch = last_event_dispatch_;
  }
}

void WindowRuntime::reset_event_state_for_record(WindowRuntimeRecord& record) {
  record.event_route.reset();
  record.last_event_result = EventResult::unhandled();
  record.last_event_dispatch.reset();
}

} // namespace cgpui
