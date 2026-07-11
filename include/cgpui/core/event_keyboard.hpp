#pragma once

#include <cstdint>

namespace cgpui {

enum class KeyAction {
  pressed,
  released,
};

struct KeyboardModifiers {
  bool shift = false;
  bool control = false;
  bool alt = false;
  bool super = false;
};

struct KeyboardKey {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
  std::uint32_t scan_code = 0;
  std::uint16_t repeat_count = 1;
  bool repeated = false;
  bool extended = false;
  bool system = false;
};

} // namespace cgpui
