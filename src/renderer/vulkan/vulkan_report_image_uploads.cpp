#include "vulkan_report_internal.hpp"

#include <utility>

namespace cgpui {

std::vector<ImageUploadBatch> vulkan_plan_image_uploads(
    std::span<const ImageAsset> assets) {
  std::vector<ImageUploadBatch> batches;
  batches.reserve(assets.size());

  for (const ImageAsset& asset : assets) {
    ImageUploadBatch batch{
        .image = describe_image_asset(asset),
    };
    batch.rgba = asset.bitmap.pixels;
    batch.uploads.push_back(ImageUploadRegion{
        .asset_id = asset.id,
        .width = asset.bitmap.width,
        .height = asset.bitmap.height,
        .stride = asset.bitmap.stride,
        .byte_offset = 0,
        .byte_size = asset.bitmap.pixels.size(),
    });
    batches.push_back(std::move(batch));
  }

  return batches;
}

} // namespace cgpui
