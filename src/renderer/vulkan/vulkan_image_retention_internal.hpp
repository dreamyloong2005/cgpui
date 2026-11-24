#pragma once

#include "vulkan_image_texture_uploads_internal.hpp"

namespace cgpui {

struct VulkanRetainedImage {
  ImageUploadBatch upload;
  std::uint64_t last_used_frame = 0;
};

void vulkan_invalidate_retained_images(
    std::vector<VulkanRetainedImage>& retained, std::span<const ImageAssetId> invalidations);
void vulkan_prune_retained_images(
    std::vector<VulkanRetainedImage>& retained, const VulkanImageTextureResources& resources);
void vulkan_retain_image_uploads(
    std::vector<VulkanRetainedImage>& retained, std::span<ImageUploadBatch> uploads,
    std::uint64_t frame_index);
void vulkan_report_retained_images(
    std::span<const VulkanRetainedImage> retained, RendererFrameDiagnosticSnapshot& snapshot);
RendererUploadByteCounts vulkan_staged_image_byte_counts(
    const VulkanImageTextureUploadResources& uploads);
Result<void> vulkan_validate_retained_image_requests(
    std::span<const VulkanRetainedImage> retained, std::span<const ImageDraw> draws,
    std::span<const ImageUploadBatch> incoming, std::span<const ImageAssetId> invalidations);

Result<void> vulkan_restore_image_resources(
    VkPhysicalDevice physical, VkDevice device, std::span<const VulkanRetainedImage> retained,
    std::span<const ImageUploadBatch> incoming, VulkanImageTextureResources& textures);
Result<void> vulkan_restore_image_uploads(
    VkPhysicalDevice physical, VkDevice device, std::span<const VulkanRetainedImage> retained,
    std::span<const ImageUploadBatch> incoming, const VulkanImageTextureResources& textures,
    VulkanImageTextureUploadResources& uploads);

} // namespace cgpui
