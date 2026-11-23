#pragma once

#include "cgpui/core/paint_values.hpp"

namespace cgpui {

struct RoundedRectRadiiResolution {
  BorderRadii radii;
  float scale = 1.0F;
  bool normalized = false;
};

[[nodiscard]] RoundedRectRadiiResolution
resolve_rounded_rect_radii(Size size, BorderRadii requested);

} // namespace cgpui
