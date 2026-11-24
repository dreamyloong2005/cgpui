#include "vulkan_image_retention_internal.hpp"

namespace cgpui {
namespace {
bool replaced(ImageAssetId id, std::span<const ImageUploadBatch> incoming) {
  return std::ranges::any_of(incoming, [id](const auto& item) { return item.image.id == id; });
}
} // namespace

Result<void> vulkan_restore_image_resources(
    VkPhysicalDevice physical, VkDevice device, std::span<const VulkanRetainedImage> retained,
    std::span<const ImageUploadBatch> incoming, VulkanImageTextureResources& textures) {
  if (std::ranges::all_of(retained,
          [&](const auto& entry) { return replaced(entry.upload.image.id, incoming); })) return {};
  if (physical == VK_NULL_HANDLE || device == VK_NULL_HANDLE)
    return std::unexpected(vulkan_error(ErrorCode::renderer_initialization_failed,
        "Vulkan image restoration requires a live device"));
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physical, &properties);
  // A replacement adapter may have smaller limits. Validate the entire restore
  // prefix before creating any resources, including images absent from this draw.
  for (const auto& entry : retained) {
    if (replaced(entry.upload.image.id, incoming)) continue;
    if (!vulkan_image_texture_upload_batch_valid(entry.upload) ||
        entry.upload.image.pixel_width > properties.limits.maxImageDimension2D ||
        entry.upload.image.pixel_height > properties.limits.maxImageDimension2D)
      return std::unexpected(vulkan_error(ErrorCode::renderer_initialization_failed,
          "retained Vulkan image exceeds the replacement device texture limit"));
  }
  for (const auto& entry : retained) {
    if (replaced(entry.upload.image.id, incoming)) continue;
    if (auto result = vulkan_ensure_image_texture_resource(
            physical, device, entry.upload.image, textures); !result) return result;
    // Restoring GPU storage is not an application use: preserve the idle age.
    vulkan_find_image_texture_resource(textures, entry.upload.image.id)->last_used_frame =
        entry.last_used_frame;
  }
  return {};
}

Result<void> vulkan_restore_image_uploads(
    VkPhysicalDevice physical, VkDevice device, std::span<const VulkanRetainedImage> retained,
    std::span<const ImageUploadBatch> incoming, const VulkanImageTextureResources& textures,
    VulkanImageTextureUploadResources& uploads) {
  for (const auto& entry : retained) {
    if (replaced(entry.upload.image.id, incoming)) continue;
    const auto* texture = vulkan_find_image_texture_resource(textures, entry.upload.image.id);
    // An invalidation, idle eviction or changed allocation must not resurrect pixels.
    if (!texture || !vulkan_image_texture_resource_matches(*texture, entry.upload.image)) continue;
    if (auto result = vulkan_stage_image_texture_uploads(
            physical, device, std::span(&entry.upload, 1), uploads); !result) return result;
  }
  return {};
}

} // namespace cgpui
