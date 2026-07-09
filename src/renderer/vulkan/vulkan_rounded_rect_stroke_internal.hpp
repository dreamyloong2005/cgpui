#pragma once

#include "cgpui/renderer/renderer_commands.hpp"

namespace cgpui {

struct VulkanRoundedRectStrokeResolution {
  bool enabled = false;
  Color color;
  float width = 0.0F;
  Rect inner_rect;
  BorderRadii inner_radii;
};

[[nodiscard]] VulkanRoundedRectStrokeResolution
vulkan_resolve_rounded_rect_stroke(
    const RoundedRectDraw& draw,
    BorderRadii normalized_outer_radii);

} // namespace cgpui
