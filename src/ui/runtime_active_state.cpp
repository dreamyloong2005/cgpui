#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::update_active_state_for_event(
    const PlatformEvent& event) {
  const auto* button = std::get_if<PointerButton>(&event);
  if (button == nullptr || button->button != MouseButton::left) {
    return;
  }

  const std::optional<ElementId> previous_active_element_id =
      active_element_id_;
  active_element_id_.reset();

  if (button->pressed && current_event_route_.has_value() &&
      current_event_route_->target_element_id.has_value()) {
    const ElementId target = *current_event_route_->target_element_id;
    const Element* element = routed_element(target);
    if (element != nullptr && element->enabled()) {
      active_element_id_ = target;
    }
  }

  request_style_state_invalidation(
      previous_active_element_id,
      active_element_id_);
}

} // namespace cgpui
