#include "vulkan_rounded_rect_stroke_internal.hpp"

#include "vulkan_rounded_rect_radii_internal.hpp"

#include <algorithm>

namespace cgpui {

VulkanRoundedRectStrokeResolution vulkan_resolve_rounded_rect_stroke(
    const RoundedRectDraw& draw,
    BorderRadii normalized_outer_radii) {
  const float maximum_width =
      std::max(0.0F, std::min(draw.rect.size.width, draw.rect.size.height) * 0.5F);
  const float width =
      std::min(std::max(0.0F, draw.border_width), maximum_width);
  if (!draw.border_color.has_value() || width <= 0.0F) {
    return VulkanRoundedRectStrokeResolution{
        .inner_rect = draw.rect,
        .inner_radii = normalized_outer_radii,
    };
  }

  const Rect inner_rect{
      .origin = {
          .x = draw.rect.origin.x + width,
          .y = draw.rect.origin.y + width,
      },
      .size = {
          .width = std::max(0.0F, draw.rect.size.width - width * 2.0F),
          .height = std::max(0.0F, draw.rect.size.height - width * 2.0F),
      },
  };
  const BorderRadii requested_inner_radii = BorderRadii::corners(
      std::max(0.0F, normalized_outer_radii.top_left - width),
      std::max(0.0F, normalized_outer_radii.top_right - width),
      std::max(0.0F, normalized_outer_radii.bottom_right - width),
      std::max(0.0F, normalized_outer_radii.bottom_left - width));
  return VulkanRoundedRectStrokeResolution{
      .enabled = true,
      .color = *draw.border_color,
      .width = width,
      .inner_rect = inner_rect,
      .inner_radii = vulkan_resolve_rounded_rect_radii(
                         inner_rect.size, requested_inner_radii)
                         .radii,
  };
}

} // namespace cgpui
