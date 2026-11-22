#pragma once

#include "vulkan_image_texture_uploads_internal.hpp"

namespace cgpui {

// Called after the previous submitted frame is idle, before preparing the
// next one. Unsubmitted uploads must survive an acquisition retry unless
// invalidated, replaced, or assigned a different texture allocation. Returned
// asset ids keep those unsubmitted targets alive through cache eviction.
std::vector<ImageAssetId> vulkan_prepare_image_upload_retirement(
    VkDevice device, std::span<const ImageDraw> draws,
    std::span<const ImageUploadBatch> incoming,
    std::span<const ImageAssetId> invalidations,
    VulkanImageTextureUploadResources& resources);

} // namespace cgpui
