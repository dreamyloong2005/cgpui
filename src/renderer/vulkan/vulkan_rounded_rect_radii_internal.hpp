#pragma once

#include "cgpui/ui/style_values.hpp"

namespace cgpui {

struct VulkanRoundedRectRadiiResolution {
  BorderRadii radii;
  float scale = 1.0F;
  bool normalized = false;
};

[[nodiscard]] VulkanRoundedRectRadiiResolution
vulkan_resolve_rounded_rect_radii(Size size, BorderRadii requested);

} // namespace cgpui
