#pragma once

#include "cgpui/core/geometry.hpp"

#include <optional>

namespace cgpui {

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

struct BoxShadow {
  Color color;
  Point offset;
  float blur_radius = 0.0F;
  float spread_radius = 0.0F;
};

// Blur of what is drawn behind a box (CSS backdrop-filter: blur()).
// `radius` is the Gaussian standard deviation in logical pixels; `tint`
// is mixed over the blurred backdrop by its alpha.
struct BackdropBlur {
  float radius = 0.0F;
  std::optional<Color> tint;
};

} // namespace cgpui
