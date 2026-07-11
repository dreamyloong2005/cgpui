#include "win32_window_internal.hpp"

#include <utility>

namespace cgpui {

void Win32Window::pointer_moved(LPARAM lparam) {
  callback_(PointerMoved{.position = Point{
      static_cast<float>(GET_X_LPARAM(lparam)),
      static_cast<float>(GET_Y_LPARAM(lparam))}});
}

void Win32Window::pointer_button(
    MouseButton button,
    bool pressed,
    std::uint8_t click_count,
    LPARAM lparam) {
  callback_(PointerButton{
      .button = button,
      .pressed = pressed,
      .click_count = click_count,
      .position = Point{
          static_cast<float>(GET_X_LPARAM(lparam)),
          static_cast<float>(GET_Y_LPARAM(lparam))}});
}

void Win32Window::pointer_scrolled(
    Point delta,
    bool precise,
    LPARAM lparam) {
  POINT point{
      .x = GET_X_LPARAM(lparam),
      .y = GET_Y_LPARAM(lparam),
  };
  ScreenToClient(hwnd_, &point);
  callback_(PointerScrolled{
      .delta = delta,
      .position = Point{
          static_cast<float>(point.x),
          static_cast<float>(point.y)},
      .precise = precise});
}

void Win32Window::key_event(KeyboardKey event) {
  callback_(std::move(event));
}

void Win32Window::text_input(WPARAM wparam) {
  const wchar_t character = static_cast<wchar_t>(wparam);
  auto text = utf8_from_utf16(std::wstring_view(&character, 1));
  if (!text.empty()) {
    callback_(TextInput{
        .text = std::move(text),
        .modifiers = current_modifiers()});
  }
}

void Win32Window::activation_changed(bool active) {
  active_ = active;
  callback_(WindowActivated{.active = active});
}

void Win32Window::focus_changed(bool focused) {
  focused_ = focused;
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
