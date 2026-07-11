#include "win32_keyboard_key_internal.hpp"

#include <cstdint>

namespace cgpui {
namespace {

bool lparam_bit(LPARAM lparam, unsigned bit) {
  return (static_cast<std::uintptr_t>(lparam) &
          (static_cast<std::uintptr_t>(1) << bit)) != 0;
}

} // namespace

std::optional<KeyboardKey> decode_win32_keyboard_key(
    UINT message,
    WPARAM wparam,
    LPARAM lparam) {
  const bool pressed = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
  const bool released = message == WM_KEYUP || message == WM_SYSKEYUP;
  if (!pressed && !released) {
    return std::nullopt;
  }
  const auto bits = static_cast<std::uintptr_t>(lparam);
  return KeyboardKey{
      .key_code = static_cast<std::uint32_t>(wparam),
      .action = pressed ? KeyAction::pressed : KeyAction::released,
      .modifiers = current_modifiers(),
      .scan_code = static_cast<std::uint32_t>((bits >> 16U) & 0xFFU),
      .repeat_count = static_cast<std::uint16_t>(bits & 0xFFFFU),
      .repeated = pressed && lparam_bit(lparam, 30U),
      .extended = lparam_bit(lparam, 24U),
      .system = message == WM_SYSKEYDOWN || message == WM_SYSKEYUP,
  };
}

} // namespace cgpui
