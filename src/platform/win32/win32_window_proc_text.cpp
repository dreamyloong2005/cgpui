#include "win32_internal.hpp"
#include "win32_text_input_internal.hpp"

namespace cgpui {

bool win32_window_proc_handle_text(
    UINT message,
    WPARAM wparam,
    Win32WindowMessageTarget* window,
    LRESULT& result) {
  const auto text = decode_win32_text_input(message, wparam);
  if (!text.has_value()) {
    return false;
  }
  if (text->probe) {
    result = TRUE;
    return true;
  }
  if (window != nullptr) {
    window->text_input(*text);
  }
  if (text->system) {
    return false;
  }
  result = 0;
  return true;
}

} // namespace cgpui
