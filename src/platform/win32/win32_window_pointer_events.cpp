#include "win32_window_internal.hpp"

#include "win32_input_coordinates_internal.hpp"

namespace cgpui {

void Win32Window::pointer_moved(LPARAM lparam) {
  callback_(PointerMoved{
      .position = win32_logical_client_point(lparam, state_.scale),
  });
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
      .position = win32_logical_client_point(lparam, state_.scale),
  });
}

void Win32Window::pointer_scrolled(
    Point delta,
    bool precise,
    LPARAM lparam) {
  callback_(PointerScrolled{
      .delta = delta,
      .position = win32_logical_client_point_from_screen(
          hwnd_,
          lparam,
          state_.scale),
      .precise = precise,
  });
}

} // namespace cgpui
