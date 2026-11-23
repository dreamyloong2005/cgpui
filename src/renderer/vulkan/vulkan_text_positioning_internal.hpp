#pragma once

#include "cgpui/renderer/planning/text_positioning.hpp"

namespace cgpui {

using VulkanTextPositioningPolicy = TextPositioningPolicy;

[[nodiscard]] VulkanTextPositioningPolicy
vulkan_default_text_positioning_policy();
[[nodiscard]] Rect vulkan_position_text_bounds(
    Rect bounds,
    VulkanTextPositioningPolicy policy);

} // namespace cgpui
