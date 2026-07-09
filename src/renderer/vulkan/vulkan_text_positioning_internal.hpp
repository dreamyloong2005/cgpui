#pragma once

#include "cgpui/renderer/glyph_atlas_types.hpp"

namespace cgpui {

enum class VulkanTextPositioningPolicy {
  preserve_subpixel,
  snap_to_device_pixels,
};

[[nodiscard]] VulkanTextPositioningPolicy
vulkan_default_text_positioning_policy();
[[nodiscard]] Rect vulkan_position_text_bounds(
    Rect bounds,
    VulkanTextPositioningPolicy policy);

} // namespace cgpui
