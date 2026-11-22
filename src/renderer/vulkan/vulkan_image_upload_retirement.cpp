#include "vulkan_image_upload_retirement_internal.hpp"

#include <algorithm>

namespace cgpui {

std::vector<ImageAssetId> vulkan_prepare_image_upload_retirement(
    VkDevice device, std::span<const ImageDraw> draws,
    std::span<const ImageUploadBatch> incoming,
    std::span<const ImageAssetId> invalidations,
    VulkanImageTextureUploadResources& resources) {
  if (resources.submitted) {
    vulkan_destroy_image_texture_upload_resources(device, resources);
    return {};
  }
  VulkanImageTextureUploadResources retired;
  std::erase_if(resources.uploads, [&](const auto& upload) {
    const bool invalidated = std::ranges::find(invalidations, upload.asset_id) !=
                             invalidations.end();
    const bool replaced = std::ranges::any_of(incoming, [&](const auto& next) {
      return next.image.id == upload.asset_id;
    });
    const bool reallocated = std::ranges::any_of(draws, [&](const auto& draw) {
      return draw.asset.id == upload.asset_id &&
             (draw.asset.pixel_width != upload.descriptor.pixel_width ||
              draw.asset.pixel_height != upload.descriptor.pixel_height ||
              draw.asset.format != upload.descriptor.format);
    });
    if (!invalidated && !replaced && !reallocated) return false;
    retired.uploads.push_back(upload);
    return true;
  });
  vulkan_destroy_image_texture_upload_resources(device, retired);
  std::vector<ImageAssetId> retained;
  retained.reserve(resources.uploads.size());
  for (const auto& upload : resources.uploads) retained.push_back(upload.asset_id);
  return retained;
}

} // namespace cgpui
