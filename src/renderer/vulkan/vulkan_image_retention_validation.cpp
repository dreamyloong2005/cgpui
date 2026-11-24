#include "vulkan_image_retention_internal.hpp"
#include "../renderer_image_validation_internal.hpp"

namespace cgpui {

Result<void> vulkan_validate_retained_image_requests(
    std::span<const VulkanRetainedImage> retained, std::span<const ImageDraw> draws,
    std::span<const ImageUploadBatch> incoming, std::span<const ImageAssetId> invalidations) {
  for (const auto& draw : draws) {
    if (std::ranges::find(invalidations, draw.asset.id) != invalidations.end() ||
        std::ranges::any_of(incoming, [&](const auto& item) { return item.image.id == draw.asset.id; }))
      continue;
    const auto entry = std::ranges::find_if(retained,
        [&](const auto& item) { return item.upload.image.id == draw.asset.id; });
    // Losing native storage does not remove the accepted allocation identity.
    if (entry != retained.end() && !renderer_same_image_allocation(draw.asset, entry->upload.image))
      return std::unexpected(vulkan_error(ErrorCode::invalid_argument,
          "Vulkan image draw descriptor does not match its retained asset"));
  }
  return {};
}

} // namespace cgpui
