#include "win32_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_pointer(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result) {
  switch (message) {
    case WM_MOUSEMOVE:
      if (window != nullptr) {
        window->pointer_moved(lparam);
      }
      result = 0;
      return true;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      if (window != nullptr) {
        window->pointer_button(
            MouseButton::left,
            message == WM_LBUTTONDOWN,
            lparam);
      }
      result = 0;
      return true;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      if (window != nullptr) {
        window->pointer_button(
            MouseButton::right,
            message == WM_RBUTTONDOWN,
            lparam);
      }
      result = 0;
      return true;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      if (window != nullptr) {
        window->pointer_button(
            MouseButton::middle,
            message == WM_MBUTTONDOWN,
            lparam);
      }
      result = 0;
      return true;
    case WM_MOUSEWHEEL:
      if (window != nullptr) {
        window->pointer_scrolled(wparam, lparam);
      }
      result = 0;
      return true;
    default:
      return false;
  }
}

} // namespace cgpui
