#pragma once

#include "cgpui/renderer/image_uploads.hpp"

namespace cgpui {

std::vector<ImageUploadBatch> plan_image_uploads(
    std::span<const ImageAsset> assets);

} // namespace cgpui
