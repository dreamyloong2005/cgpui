#pragma once

#include "cgpui/renderer/image_uploads.hpp"
#include "vulkan_image_texture_resources_internal.hpp"

#include <vector>

namespace cgpui {

struct VulkanImageTextureStagingUpload {
  ImageAssetId asset_id;
  std::size_t byte_size = 0;
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkBufferImageCopy copy_region{};
};

struct VulkanImageTextureUploadResources {
  std::vector<VulkanImageTextureStagingUpload> uploads;
};

[[nodiscard]] bool vulkan_image_texture_upload_batch_valid(
    const ImageUploadBatch& batch);
[[nodiscard]] VkBufferImageCopy vulkan_image_texture_buffer_image_copy(
    const ImageUploadRegion& upload,
    std::size_t buffer_offset);
[[nodiscard]] VkPipelineStageFlags vulkan_image_texture_upload_source_stage(
    VkImageLayout layout);
[[nodiscard]] VkImageMemoryBarrier vulkan_image_texture_upload_barrier(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout);

Result<void> vulkan_stage_image_texture_uploads(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const ImageUploadBatch> batches,
    VulkanImageTextureUploadResources& resources);
void vulkan_destroy_image_texture_upload_resources(
    VkDevice device,
    VulkanImageTextureUploadResources& resources);
Result<void> vulkan_record_image_texture_uploads(
    VkCommandBuffer command_buffer,
    const VulkanImageTextureResources& texture_resources,
    const VulkanImageTextureUploadResources& upload_resources);
void vulkan_commit_image_texture_uploads(
    VulkanImageTextureResources& texture_resources,
    const VulkanImageTextureUploadResources& upload_resources);

} // namespace cgpui
