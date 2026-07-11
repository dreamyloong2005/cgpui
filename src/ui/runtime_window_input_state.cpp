#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::sync_root_input_record() {
  WindowRuntimeRecord* record =
      find_window_runtime_record(root_window_runtime_id_);
  if (record != nullptr) {
    record->input = input_state();
  }
}

void WindowRuntime::update_input_state_for_record(
    WindowRuntimeRecord& record,
    const PlatformEvent& event) {
  if (const auto* focused = std::get_if<WindowFocused>(&event);
      focused != nullptr) {
    record.input.focused = focused->focused;
  } else if (const auto* capture = std::get_if<PointerCaptureChanged>(&event);
             capture != nullptr && !capture->captured) {
    record.input.pointer_capture_owner.reset();
    record.input.pointer_captured = false;
    record.input.pointer_down_element_id.reset();
    record.input.dragging_element_id.reset();
    record.input.active_element_id.reset();
    record.input.dragging = false;
  } else if (const std::optional<Point> position = pointer_position_for(event);
             position.has_value()) {
    record.input.pointer_position = *position;
  }
}

void WindowRuntime::capture_pointer_for_window(
    WindowRuntimeId runtime_id,
    PointerCaptureOwner owner) {
  if (runtime_id == root_window_runtime_id_) {
    capture_pointer(owner);
    return;
  }
  WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
  if (record == nullptr || !record->active || record->window == nullptr ||
      !is_valid_pointer_capture_owner(owner)) {
    return;
  }
  const bool was_captured = record->input.pointer_captured;
  record->input.pointer_capture_owner = owner;
  record->input.pointer_captured = true;
  if (!was_captured) {
    record->window->set_pointer_capture(true);
  }
}

void WindowRuntime::release_pointer_for_window(
    WindowRuntimeId runtime_id,
    PointerCaptureOwner owner) {
  if (runtime_id == root_window_runtime_id_) {
    release_pointer(owner);
    return;
  }
  WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
  if (record == nullptr || record->input.pointer_capture_owner != owner) {
    return;
  }
  record->input.pointer_capture_owner.reset();
  record->input.pointer_captured = false;
  if (record->window != nullptr) {
    record->window->set_pointer_capture(false);
  }
}

void WindowRuntime::request_keyboard_focus_for_window(
    WindowRuntimeId runtime_id,
    ViewId view_id) {
  if (runtime_id == root_window_runtime_id_) {
    request_keyboard_focus(view_id);
    return;
  }
  if (WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
      record != nullptr && view_id.value != 0) {
    record->input.keyboard_focus_owner = view_id;
    record->input.keyboard_focused = true;
  }
}

void WindowRuntime::request_keyboard_focus_for_window(
    WindowRuntimeId runtime_id,
    ElementId element_id) {
  if (runtime_id == root_window_runtime_id_) {
    request_keyboard_focus(element_id);
    return;
  }
  if (WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
      record != nullptr && element_id.value != 0) {
    record->input.keyboard_focus_element_owner = element_id;
    record->input.keyboard_focused = true;
  }
}

void WindowRuntime::release_keyboard_focus_for_window(
    WindowRuntimeId runtime_id,
    ViewId view_id) {
  if (runtime_id == root_window_runtime_id_) {
    release_keyboard_focus(view_id);
    return;
  }
  if (WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
      record != nullptr && record->input.keyboard_focus_owner == view_id) {
    record->input.keyboard_focus_owner.reset();
    record->input.keyboard_focused =
        record->input.keyboard_focus_element_owner.has_value();
  }
}

void WindowRuntime::release_keyboard_focus_for_window(
    WindowRuntimeId runtime_id,
    ElementId element_id) {
  if (runtime_id == root_window_runtime_id_) {
    release_keyboard_focus(element_id);
    return;
  }
  if (WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
      record != nullptr &&
      record->input.keyboard_focus_element_owner == element_id) {
    record->input.keyboard_focus_element_owner.reset();
    record->input.keyboard_focused =
        record->input.keyboard_focus_owner.has_value();
  }
}

} // namespace cgpui
