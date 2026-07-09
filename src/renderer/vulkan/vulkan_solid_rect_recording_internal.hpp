#pragma once

#include "vulkan_platform_internal.hpp"

namespace cgpui {

void vulkan_record_solid_rects(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    std::span<const SolidRect> rects);

} // namespace cgpui
