#include "win32_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_keyboard(
    UINT message,
    WPARAM wparam,
    Win32WindowMessageTarget* window,
    LRESULT& result) {
  switch (message) {
    case WM_KEYDOWN:
      if (window != nullptr) {
        window->key_event(wparam, KeyAction::pressed);
      }
      result = 0;
      return true;
    case WM_KEYUP:
      if (window != nullptr) {
        window->key_event(wparam, KeyAction::released);
      }
      result = 0;
      return true;
    case WM_CHAR:
      if (window != nullptr) {
        window->text_input(wparam);
      }
      result = 0;
      return true;
    default:
      return false;
  }
}

} // namespace cgpui
