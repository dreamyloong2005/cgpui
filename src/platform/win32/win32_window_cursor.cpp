#include "win32_window_internal.hpp"
#include "win32_cursor_internal.hpp"

namespace cgpui {

void Win32Window::set_cursor(CursorShape cursor_shape) {
  current_cursor_shape_ = cursor_shape;
  refresh_cursor(true);
}

void Win32Window::refresh_cursor(bool reload_system_cursor) {
  if (reload_system_cursor || current_cursor_ == nullptr) {
    current_cursor_ = load_win32_system_cursor(current_cursor_shape_);
  }
  if (hwnd_ == nullptr || current_cursor_ == nullptr) {
    return;
  }
  SetClassLongPtrW(
      hwnd_,
      GCLP_HCURSOR,
      reinterpret_cast<LONG_PTR>(current_cursor_));
  SetCursor(current_cursor_);
}

} // namespace cgpui
