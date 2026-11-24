#include "dx12_image_batch_internal.hpp"
#include <algorithm>
#include <utility>

namespace cgpui {
void Dx12ImageBatch::upload(const ImageAsset& image) {
  // Copy before replacement. Validate only the final value during preparation:
  // a later same-ID request may replace a malformed intermediate upload.
  ImageAsset owned = image;
  const auto previous = std::ranges::find_if(uploads_, [&](const ImageAsset& pending) {
    return pending.id == owned.id;
  });
  if (previous == uploads_.end()) uploads_.push_back(std::move(owned));
  else *previous = std::move(owned);
}
void Dx12ImageBatch::invalidate(ImageAssetId id) {
  if (id.value && std::ranges::find(invalidations_, id) == invalidations_.end())
    invalidations_.push_back(id);
}
} // namespace cgpui
