#include "cgpui/renderer/planning/text_positioning.hpp"

#include <cmath>

namespace cgpui {

TextPositioningPolicy default_text_positioning_policy() {
  return TextPositioningPolicy::preserve_subpixel;
}

Rect position_text_bounds(
    Rect bounds,
    TextPositioningPolicy policy) {
  if (policy == TextPositioningPolicy::preserve_subpixel) {
    return bounds;
  }

  const float left = std::round(bounds.origin.x);
  const float top = std::round(bounds.origin.y);
  const float right =
      std::round(bounds.origin.x + bounds.size.width);
  const float bottom =
      std::round(bounds.origin.y + bounds.size.height);
  return Rect{
      .origin = Point{.x = left, .y = top},
      .size = Size{.width = right - left, .height = bottom - top},
  };
}

} // namespace cgpui
