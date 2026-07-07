#include "runtime_gesture_synthesis.hpp"

#include <span>
#include <variant>

namespace cgpui {

void synthesize_pointer_gesture_state(
    ViewInputState& input,
    const PlatformEvent& event,
    std::optional<ElementId> enabled_target_element_id) {
  if (std::holds_alternative<PointerMoved>(event)) {
    input.clicked_element_id.reset();
    if (input.pointer_down_element_id.has_value()) {
      input.dragging = true;
      input.dragging_element_id = input.pointer_down_element_id;
    }
    return;
  }

  const auto* button = std::get_if<PointerButton>(&event);
  if (button == nullptr || button->button != MouseButton::left) {
    return;
  }

  input.clicked_element_id.reset();
  if (button->pressed) {
    input.pointer_down_element_id = enabled_target_element_id;
    input.dragging = false;
    input.dragging_element_id.reset();
    return;
  }

  if (input.pointer_down_element_id.has_value() &&
      enabled_target_element_id == input.pointer_down_element_id &&
      !input.dragging) {
    input.clicked_element_id = input.pointer_down_element_id;
  }
  input.pointer_down_element_id.reset();
  input.dragging = false;
  input.dragging_element_id.reset();
}

bool should_dispatch_synthesized_click_event(
    const ViewInputState& input,
    const PlatformEvent& event,
    const EventRoute& route) {
  const auto* button = std::get_if<PointerButton>(&event);
  return button != nullptr && button->button == MouseButton::left &&
         !button->pressed && input.clicked_element_id.has_value() &&
         route.target_element_id == input.clicked_element_id;
}

EventResult dispatch_synthesized_click_event(
    const PlatformEvent& event,
    const EventRoute& route,
    ElementEventContext context,
    const RuntimeGestureElementResolver& resolve_element) {
  if (!route.target_element_id.has_value()) {
    return EventResult::unhandled();
  }

  context.gesture = ElementGestureKind::click;
  const std::span<const ElementId> ancestry(route.element_ancestry);
  const auto route_ids =
      ancestry.empty()
          ? std::span<const ElementId>(&*route.target_element_id, 1)
          : ancestry;
  for (ElementId element_id : route_ids) {
    Element* element = resolve_element(element_id);
    if (element == nullptr || !element->enabled()) {
      continue;
    }

    const EventResult result = element->handle_event(event, context);
    if (result.consumed || result.cancelled) {
      return result;
    }
  }

  return EventResult::unhandled();
}

} // namespace cgpui
