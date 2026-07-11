#include "win32_window_internal.hpp"

#include <utility>

namespace cgpui {

void Win32Window::key_event(KeyboardKey event) {
  callback_(std::move(event));
}

void Win32Window::activation_changed(bool active) {
  active_ = active;
  callback_(WindowActivated{.active = active});
}

void Win32Window::focus_changed(bool focused) {
  focused_ = focused;
  if (!focused) {
    reset_win32_text_input_state(text_input_state_);
  }
  callback_(WindowFocused{.focused = focused});
}

void Win32Window::drag_entered(const Win32TestDragDropPayload* payload) {
  callback_(DragEntered{
      .position = drag_position_from_test_hook(payload),
      .payload = drag_payload_from_test_hook(payload),
      .action = drag_action_from_test_hook(payload)});
}

void Win32Window::drag_updated(const Win32TestDragDropPayload* payload) {
  callback_(DragUpdated{
      .position = drag_position_from_test_hook(payload),
      .payload = drag_payload_from_test_hook(payload),
      .action = drag_action_from_test_hook(payload)});
}

void Win32Window::drag_dropped(const Win32TestDragDropPayload* payload) {
  callback_(DragDropped{
      .position = drag_position_from_test_hook(payload),
      .payload = drag_payload_from_test_hook(payload),
      .action = drag_action_from_test_hook(payload)});
}

void Win32Window::drag_exited(const Win32TestDragDropPayload* payload) {
  callback_(DragExited{
      .position = drag_position_from_test_hook(payload),
      .payload = {},
      .action = DragDropAction::none});
}

} // namespace cgpui
