#pragma once

#include "cgpui/core/geometry.hpp"

namespace cgpui {

enum class MouseButton {
  left,
  right,
  middle,
  other,
};

enum class CursorShape {
  default_arrow,
  pointing_hand,
  text,
  crosshair,
  resize_left_right,
  resize_up_down,
  not_allowed,
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

} // namespace cgpui
