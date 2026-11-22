#include "cgpui/renderer/planning/image_uploads.hpp"

namespace cgpui {

std::vector<ImageUploadBatch> vulkan_plan_image_uploads(
    std::span<const ImageAsset> assets) {
  return plan_image_uploads(assets);
}

} // namespace cgpui
