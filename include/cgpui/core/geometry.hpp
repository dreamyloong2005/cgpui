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

struct AffineTransform {
  float scale_x = 1.0F;
  float skew_y = 0.0F;
  float skew_x = 0.0F;
  float scale_y = 1.0F;
  float translate_x = 0.0F;
  float translate_y = 0.0F;

  [[nodiscard]] static constexpr AffineTransform identity() {
    return {};
  }

  [[nodiscard]] static constexpr AffineTransform translation(
      float x,
      float y) {
    return AffineTransform{.translate_x = x, .translate_y = y};
  }

  [[nodiscard]] static constexpr AffineTransform scale(float x, float y) {
    return AffineTransform{.scale_x = x, .scale_y = y};
  }

  friend bool operator==(
      const AffineTransform&,
      const AffineTransform&) = default;
};

[[nodiscard]] constexpr AffineTransform compose(
    AffineTransform parent,
    AffineTransform child) {
  return AffineTransform{
      .scale_x = parent.scale_x * child.scale_x +
                 parent.skew_x * child.skew_y,
      .skew_y = parent.skew_y * child.scale_x +
                parent.scale_y * child.skew_y,
      .skew_x = parent.scale_x * child.skew_x +
                parent.skew_x * child.scale_y,
      .scale_y = parent.skew_y * child.skew_x +
                 parent.scale_y * child.scale_y,
      .translate_x = parent.scale_x * child.translate_x +
                     parent.skew_x * child.translate_y +
                     parent.translate_x,
      .translate_y = parent.skew_y * child.translate_x +
                     parent.scale_y * child.translate_y +
                     parent.translate_y,
  };
}

struct Color {
  float r = 0.0F;
  float g = 0.0F;
  float b = 0.0F;
  float a = 1.0F;
};

} // namespace cgpui
