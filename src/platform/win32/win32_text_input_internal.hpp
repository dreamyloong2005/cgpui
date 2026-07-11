#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace cgpui {

enum class Win32TextInputEncoding : std::uint8_t {
  utf16_code_unit,
  unicode_codepoint,
};

struct Win32TextInputMessage {
  std::uint32_t value = 0;
  Win32TextInputEncoding encoding = Win32TextInputEncoding::utf16_code_unit;
  bool system = false;
  bool probe = false;
};

struct Win32TextInputState {
  wchar_t pending_dead_key = 0;
  wchar_t pending_high_surrogate = 0;
};

struct Win32TextInputCommit {
  std::array<wchar_t, 2> utf16{};
  std::size_t length = 0;
  bool composed = false;
};

[[nodiscard]] std::optional<Win32TextInputMessage> decode_win32_text_input(
    UINT message,
    WPARAM wparam);
void record_win32_dead_key(Win32TextInputState& state, wchar_t character);
void reset_win32_text_input_state(Win32TextInputState& state);
[[nodiscard]] std::optional<Win32TextInputCommit> consume_win32_text_input(
    Win32TextInputState& state,
    Win32TextInputMessage message);

} // namespace cgpui
