#pragma once

#include "cgpui/core/geometry.hpp"

#include <optional>

namespace cgpui {

enum class Overflow {
  visible,
  hidden,
};

[[nodiscard]] constexpr float px(float value) {
  return value;
}

[[nodiscard]] constexpr Color rgb(int red, int green, int blue) {
  return Color{
      .r = static_cast<float>(red) / 255.0F,
      .g = static_cast<float>(green) / 255.0F,
      .b = static_cast<float>(blue) / 255.0F,
      .a = 1.0F,
  };
}

[[nodiscard]] constexpr Color rgba(
    int red,
    int green,
    int blue,
    float alpha) {
  return Color{
      .r = static_cast<float>(red) / 255.0F,
      .g = static_cast<float>(green) / 255.0F,
      .b = static_cast<float>(blue) / 255.0F,
      .a = alpha,
  };
}

struct EdgeSizes {
  float top = 0.0F;
  float right = 0.0F;
  float bottom = 0.0F;
  float left = 0.0F;

  [[nodiscard]] static constexpr EdgeSizes all(float value) {
    return EdgeSizes{
        .top = value,
        .right = value,
        .bottom = value,
        .left = value,
    };
  }

  [[nodiscard]] static constexpr EdgeSizes axes(
      float horizontal,
      float vertical) {
    return EdgeSizes{
        .top = vertical,
        .right = horizontal,
        .bottom = vertical,
        .left = horizontal,
    };
  }

  [[nodiscard]] static constexpr EdgeSizes trbl(
      float top,
      float right,
      float bottom,
      float left) {
    return EdgeSizes{
        .top = top,
        .right = right,
        .bottom = bottom,
        .left = left,
    };
  }
};

[[nodiscard]] constexpr EdgeSizes edges(float value) {
  return EdgeSizes::all(value);
}

[[nodiscard]] constexpr EdgeSizes edges(float horizontal, float vertical) {
  return EdgeSizes::axes(horizontal, vertical);
}

[[nodiscard]] constexpr EdgeSizes edges(
    float top,
    float right,
    float bottom,
    float left) {
  return EdgeSizes::trbl(top, right, bottom, left);
}

struct BorderRadii {
  float top_left = 0.0F;
  float top_right = 0.0F;
  float bottom_right = 0.0F;
  float bottom_left = 0.0F;

  [[nodiscard]] static constexpr BorderRadii all(float value) {
    return BorderRadii{
        .top_left = value,
        .top_right = value,
        .bottom_right = value,
        .bottom_left = value,
    };
  }

  [[nodiscard]] static constexpr BorderRadii corners(
      float top_left,
      float top_right,
      float bottom_right,
      float bottom_left) {
    return BorderRadii{
        .top_left = top_left,
        .top_right = top_right,
        .bottom_right = bottom_right,
        .bottom_left = bottom_left,
    };
  }
};

struct Style {
  std::optional<Color> background_color;
  std::optional<Color> foreground_color;
  std::optional<Color> border_color;
  std::optional<Rect> clip_rect;
  Size preferred_size;
  EdgeSizes padding;
  EdgeSizes margin;
  EdgeSizes border_width;
  BorderRadii border_radius;
  Overflow overflow = Overflow::visible;
  int z_index = 0;
  float gap = 0.0F;

  [[nodiscard]] constexpr Style with_background_color(Color color) const {
    Style style = *this;
    style.background_color = color;
    return style;
  }

  [[nodiscard]] constexpr Style with_foreground_color(Color color) const {
    Style style = *this;
    style.foreground_color = color;
    return style;
  }

  [[nodiscard]] constexpr Style with_preferred_size(Size size) const {
    Style style = *this;
    style.preferred_size = size;
    return style;
  }

  [[nodiscard]] constexpr Style with_padding(EdgeSizes edges) const {
    Style style = *this;
    style.padding = edges;
    return style;
  }

  [[nodiscard]] constexpr Style with_margin(EdgeSizes edges) const {
    Style style = *this;
    style.margin = edges;
    return style;
  }

  [[nodiscard]] constexpr Style with_border_width(EdgeSizes edges) const {
    Style style = *this;
    style.border_width = edges;
    return style;
  }

  [[nodiscard]] constexpr Style with_border_color(Color color) const {
    Style style = *this;
    style.border_color = color;
    return style;
  }

  [[nodiscard]] constexpr Style with_border_radius(BorderRadii radius) const {
    Style style = *this;
    style.border_radius = radius;
    return style;
  }

  [[nodiscard]] constexpr Style with_overflow(Overflow value) const {
    Style style = *this;
    style.overflow = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_z_index(int value) const {
    Style style = *this;
    style.z_index = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_gap(float value) const {
    Style style = *this;
    style.gap = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_clip_rect(Rect rect) const {
    Style style = *this;
    style.clip_rect = rect;
    return style;
  }
};

} // namespace cgpui
