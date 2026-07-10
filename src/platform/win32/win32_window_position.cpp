#include "win32_window_internal.hpp"

namespace cgpui {

PlatformWindowPositionState Win32Window::position_state() const {
  RECT rect{};
  if (hwnd_ == nullptr || IsWindow(hwnd_) == FALSE ||
      GetWindowRect(hwnd_, &rect) == FALSE) {
    return {};
  }
  return PlatformWindowPositionState{
      .supported = true,
      .position = Point{
          .x = static_cast<float>(rect.left),
          .y = static_cast<float>(rect.top)}};
}

bool Win32Window::request_position(Point position) {
  if (hwnd_ == nullptr || IsWindow(hwnd_) == FALSE ||
      display_command_state_.fullscreen || IsIconic(hwnd_) != FALSE ||
      IsZoomed(hwnd_) != FALSE) {
    return false;
  }
  return SetWindowPos(
             hwnd_, nullptr, static_cast<int>(position.x),
             static_cast<int>(position.y), 0, 0,
             SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE) !=
      FALSE;
}

void Win32Window::position_changed() {
  const PlatformWindowPositionState position = position_state();
  if (position.position.has_value()) {
    callback_(WindowMoved{.position = *position.position});
  }
}

} // namespace cgpui
