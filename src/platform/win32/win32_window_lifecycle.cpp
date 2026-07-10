#include "win32_window_internal.hpp"

namespace cgpui {
namespace {

PlatformWindowDisplayState display_state_for(HWND hwnd) {
  if (IsIconic(hwnd) != FALSE) {
    return PlatformWindowDisplayState::minimized;
  }
  if (IsZoomed(hwnd) != FALSE) {
    return PlatformWindowDisplayState::maximized;
  }
  return PlatformWindowDisplayState::normal;
}

} // namespace

PlatformWindowLifecycleState Win32Window::lifecycle_state() const {
  const bool created = hwnd_ != nullptr && IsWindow(hwnd_) != FALSE;
  RECT client_rect{};
  const bool configured =
      created && GetClientRect(hwnd_, &client_rect) != FALSE;
  return PlatformWindowLifecycleState{
      .native_window_created = created,
      .initial_configure_complete = configured,
      .active = created && active_,
      .focused = created && focused_,
      .close_requested = state_.close_requested,
      .display_state = created ? display_state_for(hwnd_)
                               : PlatformWindowDisplayState::normal,
  };
}

} // namespace cgpui
