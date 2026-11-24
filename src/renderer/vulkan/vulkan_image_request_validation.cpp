#include "vulkan_image_request_validation_internal.hpp"
#include "../renderer_image_validation_internal.hpp"
#include <algorithm>

namespace cgpui {
namespace {
Error invalid(const char* message) { return {.code = ErrorCode::invalid_argument, .message = message}; }
bool fits(const ImageAssetDescriptor& d, std::uint32_t maximum) {
  return d.pixel_width <= maximum && d.pixel_height <= maximum;
}
}
Result<void> vulkan_validate_image_frame_requests(
    std::span<const ImageDraw> draws, std::span<const ImageUploadBatch> uploads,
    std::span<const ImageAssetId> invalidations, const VulkanImageTextureResources& installed,
    std::uint32_t max_texture_extent) {
  if (const auto valid = vulkan_validate_image_texture_resource_requests(draws); !valid)
    return std::unexpected(Error{.code = ErrorCode::invalid_argument, .message = valid.error().message});
  for (const auto& draw : draws) {
    if (const auto valid = renderer_validate_image_draw(draw); !valid) return std::unexpected(valid.error());
    if (!fits(draw.asset, max_texture_extent))
      return std::unexpected(invalid("Vulkan image draw exceeds the device texture limit"));
  }
  for (std::size_t index = 0; index < uploads.size(); ++index) {
    const auto& upload = uploads[index];
    if (!vulkan_image_texture_upload_batch_valid(upload) || !fits(upload.image, max_texture_extent))
      return std::unexpected(invalid("image texture frame contains an invalid upload batch"));
    for (std::size_t prior = 0; prior < index; ++prior) {
      if (uploads[prior].image.id == upload.image.id)
        return std::unexpected(invalid("image texture frame contains duplicate uploads"));
    }
    for (const auto& draw : draws) {
      if (draw.asset.id == upload.image.id && !renderer_same_image_allocation(draw.asset, upload.image))
        return std::unexpected(invalid("image texture draw conflicts with its upload"));
    }
  }
  for (const auto& draw : draws) {
    const bool replaced = std::ranges::any_of(uploads, [&](const auto& upload) { return upload.image.id == draw.asset.id; });
    const bool invalidated = std::ranges::find(invalidations, draw.asset.id) != invalidations.end();
    if (replaced || invalidated) continue; // Explicit prefix owns the new identity.
    const auto* texture = vulkan_find_image_texture_resource(installed, draw.asset.id);
    if (texture && !vulkan_image_texture_resource_matches(*texture, draw.asset))
      return std::unexpected(invalid("Vulkan image draw descriptor does not match its installed asset"));
  }
  return {};
}
} // namespace cgpui
