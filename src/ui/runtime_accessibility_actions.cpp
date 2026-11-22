#include "ui_internal.hpp"

namespace cgpui {

bool WindowRuntime::handle_accessibility_action_event(
    const PlatformEvent& event,
    ViewId target_view_id,
    WindowRuntimeRecord* record) {
  const auto* action = std::get_if<AccessibilityActionRequested>(&event);
  if (action == nullptr) return false;

  state_->input.current_event_route_ = EventRouter::route_to_root(event, target_view_id);
  state_->input.current_event_route_->target_element_id = ElementId{action->element_id};
  refresh_route_ancestry(*state_->input.current_event_route_);

  EventResult result = EventResult::unhandled();
  // Additional windows resolve the element in their own tree.
  ElementTree* record_tree =
      record == nullptr ? nullptr : element_tree_for_record(*record);
  Element* element = target_view_id == state_->views.root_view_id_
      ? routed_element(ElementId{action->element_id})
      : (record_tree == nullptr ? nullptr
                                : record_tree->get(ElementId{action->element_id}));
  if (element != nullptr && element->enabled()) {
    result = element->handle_accessibility_action(
        *action,
        ElementEventContext{
            .target_element_id = ElementId{action->element_id},
            .dispatch_action = [this](std::string_view action_name) {
              return dispatch_action(std::string(action_name)).result;
            },
        });
  }

  const bool changed = result.consumed && !result.cancelled;
  if (record == nullptr) {
    finish_event_dispatch(result);
  } else {
    record->event_route = state_->input.current_event_route_;
    finish_event_dispatch_for_record(*record, result);
    state_->input.current_event_route_.reset();
  }
  if (changed && record == nullptr) {
    update_platform_accessibility_tree();
    schedule_redraw();
  } else if (changed && record->window != nullptr) {
    record->redraw_scheduled = true;
    record->window->request_redraw();
  }
  return true;
}

} // namespace cgpui
