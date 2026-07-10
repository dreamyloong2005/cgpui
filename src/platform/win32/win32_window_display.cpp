#include "win32_window_internal.hpp"

namespace cgpui {
namespace {

bool leave_fullscreen(
    HWND hwnd,
    Win32WindowDisplayCommandState& state) {
  if (!state.fullscreen) {
    return true;
  }
  state.fullscreen = false;
  SetWindowLongPtrW(hwnd, GWL_STYLE, state.windowed_style);
  SetWindowLongPtrW(hwnd, GWL_EXSTYLE, state.windowed_extended_style);
  if (!state.windowed_rect_valid) {
    return false;
  }
  const RECT rect = state.windowed_rect;
  return SetWindowPos(
             hwnd,
             nullptr,
             rect.left,
             rect.top,
             rect.right - rect.left,
             rect.bottom - rect.top,
             SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE |
                 SWP_FRAMECHANGED) != FALSE;
}

bool enter_fullscreen(
    HWND hwnd,
    Win32WindowDisplayCommandState& state) {
  if (state.fullscreen) {
    return true;
  }
  ShowWindow(hwnd, SW_RESTORE);
  state.windowed_style = GetWindowLongPtrW(hwnd, GWL_STYLE);
  state.windowed_extended_style = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  state.windowed_rect_valid = GetWindowRect(hwnd, &state.windowed_rect) != FALSE;
  MONITORINFO monitor{.cbSize = sizeof(MONITORINFO)};
  if (!state.windowed_rect_valid ||
      GetMonitorInfoW(
          MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &monitor) == FALSE) {
    return false;
  }
  state.fullscreen = true;
  SetWindowLongPtrW(
      hwnd,
      GWL_STYLE,
      state.windowed_style & ~static_cast<LONG_PTR>(WS_OVERLAPPEDWINDOW));
  if (SetWindowPos(
          hwnd,
          HWND_TOP,
          monitor.rcMonitor.left,
          monitor.rcMonitor.top,
          monitor.rcMonitor.right - monitor.rcMonitor.left,
          monitor.rcMonitor.bottom - monitor.rcMonitor.top,
          SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED) == FALSE) {
    (void)leave_fullscreen(hwnd, state);
    return false;
  }
  return true;
}

} // namespace

bool Win32Window::request_display_state(
    PlatformWindowDisplayState display_state) {
  if (hwnd_ == nullptr || IsWindow(hwnd_) == FALSE) {
    return false;
  }
  if (display_state == PlatformWindowDisplayState::fullscreen) {
    return enter_fullscreen(hwnd_, display_command_state_);
  }
  if (!leave_fullscreen(hwnd_, display_command_state_)) {
    return false;
  }
  switch (display_state) {
  case PlatformWindowDisplayState::normal:
    ShowWindow(hwnd_, SW_RESTORE);
    break;
  case PlatformWindowDisplayState::minimized:
    ShowWindow(hwnd_, SW_MINIMIZE);
    break;
  case PlatformWindowDisplayState::maximized:
    ShowWindow(hwnd_, SW_MAXIMIZE);
    break;
  case PlatformWindowDisplayState::fullscreen:
    break;
  }
  return true;
}

} // namespace cgpui
