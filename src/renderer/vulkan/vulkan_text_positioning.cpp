#include "vulkan_text_positioning_internal.hpp"

#include <cmath>

namespace cgpui {

VulkanTextPositioningPolicy vulkan_default_text_positioning_policy() {
  return VulkanTextPositioningPolicy::preserve_subpixel;
}

Rect vulkan_position_text_bounds(
    Rect bounds,
    VulkanTextPositioningPolicy policy) {
  if (policy == VulkanTextPositioningPolicy::preserve_subpixel) {
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
