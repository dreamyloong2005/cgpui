#pragma once

namespace cgpui {

struct Size {
  float width = 0.0F;
  float height = 0.0F;
};

struct Point {
  float x = 0.0F;
  float y = 0.0F;
};

struct Rect {
  Point origin;
  Size size;
};

[[nodiscard]] inline bool contains(Rect rect, Point point) {
  return point.x >= rect.origin.x &&
         point.x < rect.origin.x + rect.size.width &&
         point.y >= rect.origin.y &&
         point.y < rect.origin.y + rect.size.height;
}

struct DpiScale {
  float value = 1.0F;
};

struct Color {
  float r = 0.0F;
  float g = 0.0F;
  float b = 0.0F;
  float a = 1.0F;
};

} // namespace cgpui
