#include "win32_internal.hpp"
#include "win32_window_proc_command_internal.hpp"
#include "win32_window_proc_cursor_internal.hpp"
#include "win32_window_proc_pointer_capture_internal.hpp"
#include "win32_window_proc_accessibility_internal.hpp"
#include "win32_window_proc_background_internal.hpp"
#include "win32_window_proc_popup_internal.hpp"
#include "win32_window_proc_surface_alpha_internal.hpp"
#include "win32_native_window_work_internal.hpp"

namespace cgpui {

LRESULT CALLBACK win32_window_proc(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam) {
  auto* window = reinterpret_cast<Win32WindowMessageTarget*>(
      GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  LRESULT result = 0;
  const UINT_PTR alpha_instance = win32_window_surface_alpha_instance(hwnd);
  if (win32_window_proc_handle_native_work(hwnd, message, wparam, lparam, result) ||
      win32_window_proc_handle_surface_alpha(hwnd, message, wparam, lparam, result) ||
      win32_window_proc_handle_popup(hwnd, message, result) ||
      win32_window_proc_handle_test_drag(window, message, lparam, result) ||
      win32_window_proc_handle_accessibility(
          message, wparam, lparam, window, result) ||
      win32_window_proc_handle_command(
          message, wparam, lparam, window, result) ||
      win32_window_proc_handle_background(hwnd, message, wparam, result) ||
      win32_window_proc_handle_title_bar(
          hwnd, message, wparam, lparam, window, result) ||
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
    win32_window_proc_after_surface_alpha(hwnd, message, alpha_instance);
    return result;
  }
  result = DefWindowProcW(hwnd, message, wparam, lparam);
  win32_window_proc_after_surface_alpha(hwnd, message, alpha_instance);
  return result;
}

} // namespace cgpui
