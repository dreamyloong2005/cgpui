#pragma once

#include "cgpui/core/geometry.hpp"

#include <cstdint>
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

struct WindowCloseRequested {};

struct WindowRedrawRequested {};

struct WindowResized {
  Size size;
  DpiScale scale;
};

struct PointerMoved {
  Point position;
};

struct PointerButton {
  MouseButton button;
  bool pressed = false;
  Point position;
};

struct KeyboardKey {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
};

using PlatformEvent = std::variant<
    WindowCloseRequested,
    WindowRedrawRequested,
    WindowResized,
    PointerMoved,
    PointerButton,
    KeyboardKey>;

} // namespace cgpui
