#pragma once

#include "cgpui/renderer/image_uploads.hpp"
#include "vulkan_image_texture_resources_internal.hpp"

#include <cstdint>

namespace cgpui {

inline constexpr std::uint64_t vulkan_image_texture_cache_max_idle_frames = 120;

void vulkan_begin_image_texture_cache_frame(
    VulkanImageTextureResources& resources);
void vulkan_touch_image_texture_resource(
    VulkanImageTextureResources& resources,
    ImageAssetId asset_id);
[[nodiscard]] bool vulkan_image_texture_resource_idle(
    const VulkanImageTextureResource& resource,
    std::uint64_t frame_index,
    std::uint64_t max_idle_frames);
[[nodiscard]] std::size_t vulkan_evict_idle_image_texture_resources(
    VkDevice device,
    VulkanImageTextureResources& resources,
    std::uint64_t max_idle_frames =
        vulkan_image_texture_cache_max_idle_frames);
[[nodiscard]] std::size_t vulkan_prepare_image_texture_cache_frame(
    VkDevice device,
    std::span<const ImageDraw> image_draws,
    std::span<const ImageUploadBatch> image_uploads,
    VulkanImageTextureResources& resources);

} // namespace cgpui
