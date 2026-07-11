#include "win32_internal.hpp"
#include "win32_pointer_button_internal.hpp"
#include "win32_pointer_scroll_internal.hpp"

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
    default:
      break;
  }
  const auto scroll = decode_win32_pointer_scroll(message, wparam);
  if (scroll.has_value()) {
    if (window != nullptr) {
      window->pointer_scrolled(scroll->delta, scroll->precise, lparam);
    }
    result = 0;
    return true;
  }
  const auto button = decode_win32_pointer_button(message, wparam);
  if (!button.has_value()) {
    return false;
  }
  if (window != nullptr) {
    window->pointer_button(
        button->button,
        button->pressed,
        button->click_count,
        lparam);
  }
  result = button->result;
  return true;
}

} // namespace cgpui
