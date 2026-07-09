#pragma once

#include "cgpui/renderer/glyph_atlas_production.hpp"
#include "vulkan_glyph_atlas_resources_internal.hpp"

namespace cgpui {

struct VulkanGlyphAtlasStagingUpload {
  std::size_t page_index = 0;
  std::size_t byte_size = 0;
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  std::vector<VkBufferImageCopy> copy_regions;
};

struct VulkanGlyphAtlasUploadResources {
  std::vector<VulkanGlyphAtlasStagingUpload> uploads;
};

[[nodiscard]] VkBufferImageCopy vulkan_glyph_atlas_buffer_image_copy(
    const GlyphAtlasUploadRegion& upload,
    std::size_t buffer_offset);
[[nodiscard]] VkPipelineStageFlags vulkan_glyph_atlas_source_stage(
    VkImageLayout layout);
[[nodiscard]] VkImageMemoryBarrier vulkan_glyph_atlas_image_barrier(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout);

Result<void> vulkan_stage_glyph_atlas_uploads(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const GlyphAtlasProductionPlan& plan,
    std::span<const GlyphAtlasUploadBatch> batches,
    VulkanGlyphAtlasUploadResources& resources);
void vulkan_destroy_glyph_atlas_upload_resources(
    VkDevice device,
    VulkanGlyphAtlasUploadResources& resources);
Result<void> vulkan_record_glyph_atlas_uploads(
    VkCommandBuffer command_buffer,
    const VulkanGlyphAtlasResources& atlas_resources,
    const VulkanGlyphAtlasUploadResources& upload_resources);
void vulkan_commit_glyph_atlas_uploads(
    VulkanGlyphAtlasResources& atlas_resources,
    const VulkanGlyphAtlasUploadResources& upload_resources);

} // namespace cgpui
