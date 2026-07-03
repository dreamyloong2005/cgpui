#pragma once

#include "cgpui/renderer/renderer_types.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace cgpui {

struct ImageUploadRegion {
  ImageAssetId asset_id;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t stride = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_size = 0;
};

struct ImageUploadBatch {
  ImageAssetDescriptor image;
  std::vector<ImageUploadRegion> uploads;
  std::vector<std::uint8_t> rgba;
};

std::vector<ImageUploadBatch> vulkan_plan_image_uploads(
    std::span<const ImageAsset> assets);

} // namespace cgpui
