#include "win32_window_internal.hpp"

#include <utility>

namespace cgpui {

void Win32Window::dead_key(wchar_t character) {
  pending_dead_key_ = character;
}

void Win32Window::text_input(WPARAM wparam) {
  const wchar_t character = static_cast<wchar_t>(wparam);
  const bool composed = pending_dead_key_ != 0;
  pending_dead_key_ = 0;
  auto text = utf8_from_utf16(std::wstring_view(&character, 1));
  if (!text.empty()) {
    callback_(TextInput{
        .text = std::move(text),
        .modifiers = current_modifiers(),
        .composed = composed});
  }
}

} // namespace cgpui
