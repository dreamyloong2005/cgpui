#pragma once

#include "cgpui/renderer/planning/rect_antialiasing.hpp"

namespace cgpui {

using VulkanRoundedRectAntialiasingMode = RectAntialiasingMode;

using VulkanRoundedRectAntialiasingPolicy = RectAntialiasingPolicy;

[[nodiscard]] VulkanRoundedRectAntialiasingPolicy
vulkan_default_rounded_rect_antialiasing_policy();
[[nodiscard]] float vulkan_rounded_rect_coverage(float coverage);

} // namespace cgpui
