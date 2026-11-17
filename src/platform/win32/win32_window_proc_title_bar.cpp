#include "win32_window_title_bar_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_title_bar(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam,
    Win32TitleBarMessageTarget* window, LRESULT& result) {
  switch (message) {
    case WM_NCCALCSIZE:
    case WM_NCHITTEST:
    case WM_NCMOUSEMOVE:
    case WM_NCMOUSELEAVE:
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONDBLCLK:
    case WM_NCLBUTTONUP:
      return window != nullptr &&
          window->title_bar_message(hwnd, message, wparam, lparam, result);
    default:
      return false;
  }
}

} // namespace cgpui
