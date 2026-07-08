#include "ui_internal.hpp"

namespace cgpui {
namespace {

[[nodiscard]] const Element* find_static_element(
    const Element* element,
    ElementId element_id) {
  if (element == nullptr) {
    return nullptr;
  }
  if (element->id() == element_id) {
    return element;
  }
  if (const auto* stack = dynamic_cast<const VerticalStackElement*>(element)) {
    for (const auto& child : stack->children()) {
      if (const Element* found = find_static_element(child.get(), element_id)) {
        return found;
      }
    }
  } else if (const auto* flex = dynamic_cast<const FlexElement*>(element)) {
    for (const auto& child : flex->children()) {
      if (const Element* found = find_static_element(child.get(), element_id)) {
        return found;
      }
    }
  } else if (const auto* styled = dynamic_cast<const StyledElement*>(element)) {
    for (const auto& child : styled->children()) {
      if (const Element* found = find_static_element(child.get(), element_id)) {
        return found;
      }
    }
  } else if (const auto* focus = dynamic_cast<const FocusableElement*>(element)) {
    return find_static_element(focus->child(), element_id);
  } else if (const auto* key = dynamic_cast<const KeyElement*>(element)) {
    return find_static_element(key->child(), element_id);
  } else if (const auto* button = dynamic_cast<const ButtonElement*>(element)) {
    return find_static_element(button->child(), element_id);
  } else if (const auto* scroll = dynamic_cast<const ScrollElement*>(element)) {
    return find_static_element(scroll->child(), element_id);
  } else if (const auto* list =
                 dynamic_cast<const ScrollableListElement*>(element)) {
    return find_static_element(&list->content(), element_id);
  }
  return nullptr;
}

[[nodiscard]] bool disabled_or_missing(const WindowRuntime& runtime, ElementId element_id) {
  if (const cgpui::StaticElementTreeView* static_tree =
          runtime.static_element_tree();
      static_tree != nullptr) {
    const cgpui::StaticElementNode* node = static_tree->get(element_id);
    return node == nullptr || !node->enabled;
  }

  const Element* element = nullptr;
  if (runtime.element_tree() != nullptr) {
    element = runtime.element_tree()->get(element_id);
  } else if (runtime.element_root() != nullptr) {
    element = find_static_element(runtime.element_root(), element_id);
  } else {
    return false;
  }
  if (element == nullptr) {
    return true;
  }
  return !element->enabled();
}

} // namespace

void WindowRuntime::refresh_disabled_interaction_state() {
  if (hovered_element_id_.has_value() &&
      disabled_or_missing(*this, *hovered_element_id_)) {
    const std::optional<ElementId> previous = hovered_element_id_;
    hovered_element_id_.reset();
    cursor_shape_ = CursorShape::default_arrow;
    apply_cursor_shape(cursor_shape_);
    request_style_state_invalidation(previous, hovered_element_id_);
  }

  if (active_element_id_.has_value() &&
      disabled_or_missing(*this, *active_element_id_)) {
    const std::optional<ElementId> previous = active_element_id_;
    active_element_id_.reset();
    request_style_state_invalidation(previous, active_element_id_);
  }

  if (keyboard_focus_element_owner_.has_value() &&
      disabled_or_missing(*this, *keyboard_focus_element_owner_)) {
    const std::optional<ElementId> previous = keyboard_focus_element_owner_;
    keyboard_focus_element_owner_.reset();
    request_style_state_invalidation(previous, keyboard_focus_element_owner_);
    apply_focused_text_ime_placement();
  }

  if (pointer_capture_owner_.has_value() &&
      pointer_capture_owner_->is_element() &&
      disabled_or_missing(*this, *pointer_capture_owner_->element_id())) {
    pointer_capture_owner_.reset();
  }

  if (input_.pointer_down_element_id.has_value() &&
      disabled_or_missing(*this, *input_.pointer_down_element_id)) {
    input_.pointer_down_element_id.reset();
    input_.dragging = false;
  }
  if (input_.clicked_element_id.has_value() &&
      disabled_or_missing(*this, *input_.clicked_element_id)) {
    input_.clicked_element_id.reset();
  }
  if (input_.dragging_element_id.has_value() &&
      disabled_or_missing(*this, *input_.dragging_element_id)) {
    input_.dragging_element_id.reset();
    input_.dragging = false;
  }
}

} // namespace cgpui
