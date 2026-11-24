#pragma once
#include "vulkan_frame_draw_order_internal.hpp"

namespace cgpui {
std::vector<std::size_t> vulkan_submitted_image_indices(
    VkExtent2D extent, std::span<const VulkanFrameDrawOrderEntry> draw_order,
    std::span<const VulkanImageDrawCommand> image_commands);
} // namespace cgpui
