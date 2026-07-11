#include "win32_internal.hpp"
#include "win32_window_proc_command_internal.hpp"
#include "win32_window_proc_cursor_internal.hpp"
#include "win32_window_proc_pointer_capture_internal.hpp"

namespace cgpui {

LRESULT CALLBACK win32_window_proc(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam) {
  auto* window = reinterpret_cast<Win32WindowMessageTarget*>(
      GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  LRESULT result = 0;
  if (win32_window_proc_handle_test_drag(window, message, lparam, result) ||
      win32_window_proc_handle_command(
          message, wparam, lparam, window, result) ||
      win32_window_proc_handle_lifecycle(
          hwnd,
          message,
          wparam,
          lparam,
          window,
          result) ||
      win32_window_proc_handle_pointer_capture(
          hwnd, message, window, result) ||
      win32_window_proc_handle_pointer(
          message,
          wparam,
          lparam,
          window,
          result) ||
      win32_window_proc_handle_cursor(
          message, wparam, lparam, window, result) ||
      win32_window_proc_handle_keyboard(
          message, wparam, lparam, window, result) ||
      win32_window_proc_handle_text(message, wparam, window, result)) {
    return result;
  }
  return DefWindowProcW(hwnd, message, wparam, lparam);
}

} // namespace cgpui
