#pragma once

#include "cgpui/core/geometry.hpp"

#include <cstdint>

namespace cgpui {

enum class MouseButton {
  left,
  right,
  middle,
  back,
  forward,
  other,
};

enum class CursorShape {
  default_arrow,
  pointing_hand,
  text,
  crosshair,
  resize_left_right,
  resize_up_down,
  resize_north_west_south_east,
  resize_north_east_south_west,
  resize_all,
  wait,
  progress,
  help,
  up_arrow,
  not_allowed,
};

struct PointerMoved {
  Point position;
};

struct PointerButton {
  MouseButton button;
  bool pressed = false;
  std::uint8_t click_count = 1;
  Point position;
};

struct PointerScrolled {
  Point delta;
  Point position;
  bool precise = false;
};

} // namespace cgpui
