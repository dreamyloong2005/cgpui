#pragma once
#include "vulkan_image_texture_uploads_internal.hpp"

namespace cgpui {
Result<void> vulkan_validate_image_frame_requests(
    std::span<const ImageDraw> draws, std::span<const ImageUploadBatch> uploads,
    std::span<const ImageAssetId> invalidations, const VulkanImageTextureResources& installed,
    std::uint32_t max_texture_extent);
} // namespace cgpui
