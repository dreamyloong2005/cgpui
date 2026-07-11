#include "win32_window_proc_cursor_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_cursor(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result) {
  if (message == WM_SETCURSOR && LOWORD(lparam) == HTCLIENT) {
    if (window != nullptr) {
      window->refresh_cursor(false);
    }
    result = TRUE;
    return true;
  }
  if (message == WM_THEMECHANGED ||
      (message == WM_SETTINGCHANGE && wparam == SPI_SETCURSORS)) {
    if (window != nullptr) {
      window->refresh_cursor(true);
    }
  }
  return false;
}

} // namespace cgpui
