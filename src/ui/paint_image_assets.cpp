#include "cgpui/ui/paint.hpp"

#include <algorithm>

namespace cgpui {

void PaintList::upload_image(const ImageAsset& image) {
  if (image.id.value == 0 || image.bitmap.pixels.empty()) return;
  const auto existing = std::ranges::find_if(
      image_uploads_,
      [&](const ImageAsset* candidate) {
        return candidate != nullptr && candidate->id == image.id;
      });
  if (existing == image_uploads_.end()) {
    image_uploads_.push_back(&image);
  } else {
    *existing = &image;
  }
}

void PaintList::invalidate_image(ImageAssetId asset_id) {
  if (asset_id.value != 0 &&
      std::ranges::find(image_invalidations_, asset_id) ==
          image_invalidations_.end()) {
    image_invalidations_.push_back(asset_id);
  }
}

std::span<const ImageAsset* const> PaintList::image_uploads() const {
  return image_uploads_;
}

std::span<const ImageAssetId> PaintList::image_invalidations() const {
  return image_invalidations_;
}

} // namespace cgpui
