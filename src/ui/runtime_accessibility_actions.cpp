#include "ui_internal.hpp"

namespace cgpui {

bool WindowRuntime::handle_accessibility_action_event(
    const PlatformEvent& event,
    ViewId target_view_id,
    WindowRuntimeRecord* record) {
  const auto* action = std::get_if<AccessibilityActionRequested>(&event);
  if (action == nullptr) return false;

  current_event_route_ = EventRouter::route_to_root(event, target_view_id);
  current_event_route_->target_element_id = ElementId{action->element_id};
  refresh_route_ancestry(*current_event_route_);

  EventResult result = EventResult::unhandled();
  Element* element = target_view_id == root_view_id_
      ? routed_element(ElementId{action->element_id})
      : nullptr;
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
    record->event_route = current_event_route_;
    finish_event_dispatch_for_record(*record, result);
    current_event_route_.reset();
  }
  if (changed && record == nullptr) {
    update_platform_accessibility_tree();
    schedule_redraw();
  }
  return true;
}

} // namespace cgpui
