#include "win32_internal.hpp"
#include "win32_dead_key_internal.hpp"
#include "win32_keyboard_key_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_keyboard(
    UINT message,
    WPARAM wparam,
    LPARAM lparam,
    Win32WindowMessageTarget* window,
    LRESULT& result) {
  const auto key = decode_win32_keyboard_key(message, wparam, lparam);
  if (key.has_value()) {
    if (window != nullptr) {
      window->key_event(*key);
    }
    if (key->system) {
      return false;
    }
    result = 0;
    return true;
  }
  const auto dead_key = decode_win32_dead_key(message, wparam);
  if (dead_key.has_value()) {
    if (window != nullptr) {
      window->dead_key(dead_key->character);
    }
    if (dead_key->system) {
      return false;
    }
    result = 0;
    return true;
  }
  switch (message) {
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
