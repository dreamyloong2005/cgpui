#pragma once

#include "cgpui/core/geometry.hpp"

#include <cstdint>
#include <string>
#include <variant>

namespace cgpui {

enum class MouseButton {
  left,
  right,
  middle,
  other,
};

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

struct WindowCloseRequested {};

struct WindowRedrawRequested {};

struct WindowResized {
  Size size;
  DpiScale scale;
};

struct WindowFocused {
  bool focused = false;
};

struct PointerMoved {
  Point position;
};

struct PointerButton {
  MouseButton button;
  bool pressed = false;
  Point position;
};

struct PointerScrolled {
  Point delta;
  Point position;
};

struct KeyboardKey {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
};

struct TextInput {
  std::string text;
  KeyboardModifiers modifiers;
};

using PlatformEvent = std::variant<
    WindowCloseRequested,
    WindowRedrawRequested,
    WindowResized,
    WindowFocused,
    PointerMoved,
    PointerButton,
    PointerScrolled,
    KeyboardKey,
    TextInput>;

} // namespace cgpui
