#pragma once

#include "cgpui/core/geometry.hpp"

#include <optional>

namespace cgpui {

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

struct Style {
  std::optional<Color> background_color;
  std::optional<Color> foreground_color;
  Size preferred_size;
  EdgeSizes padding;
  EdgeSizes border_width;

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

  [[nodiscard]] constexpr Style with_border_width(EdgeSizes edges) const {
    Style style = *this;
    style.border_width = edges;
    return style;
  }
};

} // namespace cgpui
