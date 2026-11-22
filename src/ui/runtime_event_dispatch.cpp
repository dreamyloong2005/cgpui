#include "ui_internal.hpp"

namespace cgpui {

EventResult WindowRuntime::dispatch_current_event_route(
    const PlatformEvent& event) {
  if (!state_->input.current_event_route_.has_value()) {
    return EventResult::unhandled();
  }

  const bool text_selection_consumed =
      apply_text_pointer_selection(event, *state_->input.current_event_route_);
  EventResult result = text_selection_consumed ? EventResult::consumed_event()
                                               : EventResult::unhandled();
  if (!result.consumed && !result.cancelled) {
    if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
        scrolled != nullptr) {
      if (ScrollState* state = scroll_state_for_route(*state_->input.current_event_route_);
          state != nullptr) {
        state->scroll_by(scrolled->delta);
        result = EventResult::consumed_event();
      }
    }
  }
  if (!result.consumed && !result.cancelled) {
    result = dispatch_routed_element_event(event, *state_->input.current_event_route_);
  }
  if (!result.consumed && !result.cancelled) {
    state_->input.dispatching_view_event_ = true;
    result = state_->core.view_.handle_event(event, context());
    state_->input.dispatching_view_event_ = false;
  }
  return result;
}

void WindowRuntime::finish_event_dispatch(EventResult result) {
  if (!state_->input.current_event_route_.has_value()) {
    return;
  }

  state_->input.last_event_result_ = result;
  state_->diagnostics.last_event_dispatch_ = EventDispatchRecord{
      .sequence = ++state_->diagnostics.event_dispatch_sequence_,
      .view_id = state_->input.current_event_route_->target_view_id,
      .event_kind = state_->input.current_event_route_->event_kind,
      .route = *state_->input.current_event_route_,
      .result = state_->input.last_event_result_};
  sync_root_event_record();
  sync_root_input_record();
  if (state_->core.after_event_callback_) {
    state_->core.after_event_callback_(context(), *state_->diagnostics.last_event_dispatch_);
  }
  apply_focused_text_ime_placement();
  drain_deferred_callbacks();
  flush_deferred_redraw_request();
}

} // namespace cgpui
