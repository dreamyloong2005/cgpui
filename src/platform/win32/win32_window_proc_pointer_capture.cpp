#include "win32_window_proc_pointer_capture_internal.hpp"

#include "win32_pointer_capture_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_pointer_capture(
    HWND hwnd,
    UINT message,
    Win32WindowMessageTarget* window,
    LRESULT& result) {
  switch (message) {
    case WM_CAPTURECHANGED:
      if (window != nullptr) {
        window->pointer_capture_lost();
      }
      result = 0;
      return true;
    case WM_CANCELMODE:
      if (!release_win32_pointer(hwnd) && window != nullptr) {
        window->pointer_capture_lost();
      }
      result = 0;
      return true;
    default:
      return false;
  }
}

} // namespace cgpui
