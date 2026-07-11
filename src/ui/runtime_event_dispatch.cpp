#include "ui_internal.hpp"

namespace cgpui {

EventResult WindowRuntime::dispatch_current_event_route(
    const PlatformEvent& event) {
  if (!current_event_route_.has_value()) {
    return EventResult::unhandled();
  }

  const bool text_selection_consumed =
      apply_text_pointer_selection(event, *current_event_route_);
  EventResult result = text_selection_consumed ? EventResult::consumed_event()
                                               : EventResult::unhandled();
  if (!result.consumed && !result.cancelled) {
    if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
        scrolled != nullptr) {
      if (ScrollState* state = scroll_state_for_route(*current_event_route_);
          state != nullptr) {
        state->scroll_by(scrolled->delta);
        result = EventResult::consumed_event();
      }
    }
  }
  if (!result.consumed && !result.cancelled) {
    result = dispatch_routed_element_event(event, *current_event_route_);
  }
  if (!result.consumed && !result.cancelled) {
    dispatching_view_event_ = true;
    result = view_.handle_event(event, context());
    dispatching_view_event_ = false;
  }
  return result;
}

void WindowRuntime::finish_event_dispatch(EventResult result) {
  if (!current_event_route_.has_value()) {
    return;
  }

  last_event_result_ = result;
  last_event_dispatch_ = EventDispatchRecord{
      .sequence = ++event_dispatch_sequence_,
      .view_id = current_event_route_->target_view_id,
      .event_kind = current_event_route_->event_kind,
      .route = *current_event_route_,
      .result = last_event_result_};
  sync_root_input_record();
  if (after_event_callback_) {
    after_event_callback_(context(), *last_event_dispatch_);
  }
  apply_focused_text_ime_placement();
  drain_deferred_callbacks();
  flush_deferred_redraw_request();
}

} // namespace cgpui
