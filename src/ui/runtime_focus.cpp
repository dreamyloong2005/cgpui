#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::request_keyboard_focus() {
  request_keyboard_focus(root_view_id_);
}

void WindowRuntime::request_keyboard_focus(ViewId view_id) {
  keyboard_focus_owner_ = view_id;
  sync_root_input_record();
  apply_focused_text_ime_placement();
}

void WindowRuntime::request_keyboard_focus(ElementId element_id) {
  if (element_id.value != 0) {
    const std::optional<ElementId> previous_focused_element_id =
        keyboard_focus_element_owner_;
    keyboard_focus_element_owner_ = element_id;
    request_style_state_invalidation(
        previous_focused_element_id,
        keyboard_focus_element_owner_);
    sync_root_input_record();
    apply_focused_text_ime_placement();
  }
}

void WindowRuntime::release_keyboard_focus() {
  release_keyboard_focus(root_view_id_);
}

void WindowRuntime::release_keyboard_focus(ViewId view_id) {
  if (keyboard_focus_owner_ == view_id) {
    keyboard_focus_owner_.reset();
    sync_root_input_record();
    apply_focused_text_ime_placement();
  }
}

void WindowRuntime::release_keyboard_focus(ElementId element_id) {
  if (keyboard_focus_element_owner_ == element_id) {
    const std::optional<ElementId> previous_focused_element_id =
        keyboard_focus_element_owner_;
    keyboard_focus_element_owner_.reset();
    request_style_state_invalidation(
        previous_focused_element_id,
        keyboard_focus_element_owner_);
    sync_root_input_record();
    apply_focused_text_ime_placement();
  }
}

FocusHandle WindowRuntime::focus_handle(ElementId element_id) const {
  return FocusHandle(element_id);
}

ViewInputState WindowRuntime::input_state() const {
  ViewInputState input = input_;
  input.pointer_capture_owner = pointer_capture_owner_;
  input.pointer_captured = pointer_capture_owner_.has_value();
  input.keyboard_focus_owner = keyboard_focus_owner_;
  input.keyboard_focus_element_owner = keyboard_focus_element_owner_;
  input.hovered_element_id = hovered_element_id_;
  input.active_element_id = active_element_id_;
  input.cursor_shape = cursor_shape_;
  input.keyboard_focused = keyboard_focus_owner_ == root_view_id_ ||
                           keyboard_focus_element_owner_.has_value();
  return input;
}

void WindowRuntime::set_element_cursor(
    ElementId element_id,
    CursorShape cursor_shape) {
  if (element_id.value == 0) {
    return;
  }
  element_cursors_[element_id.value] = cursor_shape;
}

} // namespace cgpui
