#pragma once

#include "vulkan_image_texture_resources_internal.hpp"

namespace cgpui {

[[nodiscard]] std::size_t vulkan_invalidate_image_texture_resources(
    VkDevice device,
    std::span<const ImageAssetId> invalidations,
    VulkanImageTextureResources& resources);

} // namespace cgpui
