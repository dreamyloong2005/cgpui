#include "ui_internal.hpp"
#include "runtime_gesture_synthesis.hpp"

namespace cgpui {

void WindowRuntime::update_active_state_for_event(
    const PlatformEvent& event) {
  const bool pointer_move = std::holds_alternative<PointerMoved>(event);
  const auto* button = std::get_if<PointerButton>(&event);
  if (!pointer_move &&
      (button == nullptr || button->button != MouseButton::left)) {
    return;
  }

  std::optional<ElementId> enabled_target_element_id;
  if (current_event_route_.has_value() &&
      current_event_route_->target_element_id.has_value()) {
    const ElementId target = *current_event_route_->target_element_id;
    if (element_enabled(target)) {
      enabled_target_element_id = target;
    }
  }

  const std::optional<ElementId> previous_active_element_id =
      active_element_id_;
  if (button != nullptr) {
    active_element_id_.reset();
    if (button->pressed && enabled_target_element_id.has_value()) {
      active_element_id_ = *enabled_target_element_id;
    }
    request_style_state_invalidation(
        previous_active_element_id,
        active_element_id_);
  }

  synthesize_pointer_gesture_state(
      input_,
      event,
      enabled_target_element_id);
}

} // namespace cgpui
