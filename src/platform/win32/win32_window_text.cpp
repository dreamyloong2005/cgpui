#include "win32_window_internal.hpp"

#include <utility>

namespace cgpui {

void Win32Window::dead_key(wchar_t character) {
  record_win32_dead_key(text_input_state_, character);
}

void Win32Window::text_input(Win32TextInputMessage message) {
  const auto commit = consume_win32_text_input(text_input_state_, message);
  if (!commit.has_value()) {
    return;
  }
  auto text = utf8_from_utf16(
      std::wstring_view(commit->utf16.data(), commit->length));
  if (!text.empty()) {
    callback_(TextInput{
        .text = std::move(text),
        .modifiers = current_modifiers(),
        .composed = commit->composed});
  }
}

} // namespace cgpui
