#include "win32_text_input_internal.hpp"

namespace cgpui {
namespace {

bool is_high_surrogate(std::uint32_t value) {
  return value >= 0xD800U && value <= 0xDBFFU;
}

bool is_low_surrogate(std::uint32_t value) {
  return value >= 0xDC00U && value <= 0xDFFFU;
}

bool is_control_character(std::uint32_t value) {
  return value < 0x20U || value == 0x7FU;
}

Win32TextInputCommit commit_utf16(
    Win32TextInputState& state,
    wchar_t first,
    wchar_t second = 0) {
  Win32TextInputCommit commit{
      .utf16 = {first, second},
      .length = second == 0 ? 1U : 2U,
      .composed = state.pending_dead_key != 0};
  state.pending_dead_key = 0;
  state.pending_high_surrogate = 0;
  return commit;
}

} // namespace

std::optional<Win32TextInputMessage> decode_win32_text_input(
    UINT message,
    WPARAM wparam) {
  if (message == WM_CHAR || message == WM_SYSCHAR) {
    return Win32TextInputMessage{
        .value = static_cast<std::uint32_t>(wparam),
        .system = message == WM_SYSCHAR};
  }
  if (message == WM_UNICHAR) {
    return Win32TextInputMessage{
        .value = static_cast<std::uint32_t>(wparam),
        .encoding = Win32TextInputEncoding::unicode_codepoint,
        .probe = wparam == UNICODE_NOCHAR};
  }
  return std::nullopt;
}

void record_win32_dead_key(Win32TextInputState& state, wchar_t character) {
  state.pending_dead_key = character;
}

void reset_win32_text_input_state(Win32TextInputState& state) {
  state = {};
}

std::optional<Win32TextInputCommit> consume_win32_text_input(
    Win32TextInputState& state,
    Win32TextInputMessage message) {
  if (message.system) {
    reset_win32_text_input_state(state);
    return std::nullopt;
  }
  if (is_control_character(message.value)) {
    reset_win32_text_input_state(state);
    return std::nullopt;
  }
  if (message.encoding == Win32TextInputEncoding::unicode_codepoint) {
    state.pending_high_surrogate = 0;
    if (message.value > 0x10FFFFU || is_high_surrogate(message.value) ||
        is_low_surrogate(message.value)) {
      state.pending_dead_key = 0;
      return std::nullopt;
    }
    if (message.value <= 0xFFFFU) {
      return commit_utf16(state, static_cast<wchar_t>(message.value));
    }
    const std::uint32_t value = message.value - 0x10000U;
    return commit_utf16(
        state,
        static_cast<wchar_t>(0xD800U + (value >> 10U)),
        static_cast<wchar_t>(0xDC00U + (value & 0x3FFU)));
  }

  if (message.value > 0xFFFFU) {
    reset_win32_text_input_state(state);
    return std::nullopt;
  }
  const auto code_unit = static_cast<wchar_t>(message.value);
  if (is_high_surrogate(message.value)) {
    state.pending_high_surrogate = code_unit;
    return std::nullopt;
  }
  if (is_low_surrogate(message.value)) {
    if (state.pending_high_surrogate == 0) {
      state.pending_dead_key = 0;
      return std::nullopt;
    }
    return commit_utf16(state, state.pending_high_surrogate, code_unit);
  }
  state.pending_high_surrogate = 0;
  return commit_utf16(state, code_unit);
}

} // namespace cgpui
