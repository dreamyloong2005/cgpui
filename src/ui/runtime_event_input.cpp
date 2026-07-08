#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::update_input_state_for_event(const PlatformEvent& event) {
  if (const auto* focused = std::get_if<WindowFocused>(&event);
      focused != nullptr) {
    input_.focused = focused->focused;
  } else if (const auto* moved = std::get_if<PointerMoved>(&event);
             moved != nullptr) {
    input_.pointer_position = moved->position;
  } else if (const auto* button = std::get_if<PointerButton>(&event);
             button != nullptr) {
    input_.pointer_position = button->position;
  } else if (const auto* scrolled = std::get_if<PointerScrolled>(&event);
             scrolled != nullptr) {
    input_.pointer_position = scrolled->position;
  } else if (const std::optional<Point> drag_position =
                 pointer_position_for(event);
             drag_position.has_value() &&
             (std::holds_alternative<DragEntered>(event) ||
              std::holds_alternative<DragUpdated>(event) ||
              std::holds_alternative<DragDropped>(event) ||
              std::holds_alternative<DragExited>(event))) {
    input_.pointer_position = *drag_position;
  }

  if (std::holds_alternative<DragEntered>(event) ||
      std::holds_alternative<DragUpdated>(event) ||
      std::holds_alternative<DragDropped>(event) ||
      std::holds_alternative<DragExited>(event)) {
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::drag_drop,
        .event_kind = event_kind_for(event),
        .backend = "runtime",
        .operation = drag_drop_operation_for(event),
        .supported = true,
        .succeeded = true,
        .value_count = drag_drop_payload_value_count(event),
    });
  }
}

std::optional<ElementId> WindowRuntime::hit_test_target_for_event(
    const PlatformEvent& event) {
  const std::optional<Point> pointer_position = pointer_position_for(event);
  if (!pointer_position.has_value()) {
    return {};
  }

  if (static_element_tree_installed()) {
    const ElementId hit = static_element_tree_.hit_test(*pointer_position);
    return hit.value == 0 ? std::optional<ElementId>{}
                          : std::optional<ElementId>{hit};
  }

  if (element_root() == nullptr) {
    return {};
  }

  const ElementId hit = hit_test_runtime_element_root(
      owned_element_tree_.get(),
      element_root_,
      *pointer_position);
  if (hit.value == 0) {
    return {};
  }
  return hit;
}

void WindowRuntime::update_hover_cursor_for_event(
    const PlatformEvent& event,
    std::optional<ElementId> hit_element_id) {
  if (!std::holds_alternative<PointerMoved>(event)) {
    return;
  }

  const std::optional<ElementId> previous_hovered_element_id =
      hovered_element_id_;
  hovered_element_id_.reset();
  cursor_shape_ = CursorShape::default_arrow;
  if (static_element_tree_installed() || element_root() != nullptr) {
    hovered_element_id_ = hit_element_id;
    if (hit_element_id.has_value()) {
      const Element* hovered_element = routed_element(*hit_element_id);
      const bool enabled =
          static_element_tree_installed()
              ? element_enabled(*hit_element_id)
              : (hovered_element == nullptr || hovered_element->enabled());
      if (enabled) {
        if (const auto cursor = element_cursors_.find(hit_element_id->value);
            cursor != element_cursors_.end()) {
          cursor_shape_ = cursor->second;
        }
      }
    }
  }
  apply_cursor_shape(cursor_shape_);
  request_style_state_invalidation(
      previous_hovered_element_id,
      hovered_element_id_);
}

void WindowRuntime::resolve_event_route_target(
    const PlatformEvent& event,
    std::optional<ElementId> hit_element_id) {
  if (!current_event_route_.has_value()) {
    return;
  }

  if (keyboard_focus_element_owner_.has_value() &&
      is_keyboard_routed_event(event)) {
    current_event_route_->target_element_id = keyboard_focus_element_owner_;
  } else if (pointer_capture_owner_.has_value() &&
             pointer_position_for(event).has_value()) {
    apply_pointer_capture_owner_to_route(
        *pointer_capture_owner_,
        *current_event_route_);
  } else if (hit_element_id.has_value()) {
    current_event_route_->target_element_id = hit_element_id;
  }
}

void WindowRuntime::apply_focus_activation_for_event(
    const PlatformEvent& event) {
  if (!current_event_route_.has_value() || !is_focus_activation_event(event) ||
      !current_event_route_->target_element_id.has_value()) {
    return;
  }

  if (!element_focusable(*current_event_route_->target_element_id)) {
    return;
  }

  request_keyboard_focus(*current_event_route_->target_element_id);
  if (Element* element = routed_element(*current_event_route_->target_element_id);
      element != nullptr) {
    element->focus(ElementFocusContext{
        .element_id = *current_event_route_->target_element_id,
    });
  }
}

} // namespace cgpui
