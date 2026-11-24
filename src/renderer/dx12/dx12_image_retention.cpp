#include "dx12_image_retention_internal.hpp"
#include <algorithm>
#include <limits>
#include <new>
#include <stdexcept>

namespace cgpui {
void dx12_add_retained_image_bytes(Dx12ImageRetentionSample& sample, std::size_t bytes) noexcept {
  if (!sample.byte_count) return;
  const auto maximum = std::numeric_limits<std::size_t>::max();
  if (bytes > maximum - *sample.byte_count) {
    sample.byte_count = maximum;
    sample.saturated = true;
  } else {
    *sample.byte_count += bytes;
  }
}

void Dx12ImageRetention::prune() noexcept {
  std::erase_if(images_, [](const auto& image) { return image.expired(); });
}

void Dx12ImageRetention::track(const Dx12TextureHandle& image) noexcept {
  prune();
  if (!complete_ || !image || image->format != DXGI_FORMAT_R8G8B8A8_UNORM) return;
  for (const auto& known : images_) {
    if (!known.owner_before(image) && !image.owner_before(known)) return;
  }
  try {
    images_.emplace_back(image);
  } catch (const std::bad_alloc&) {
    complete_ = false;
  } catch (const std::length_error&) {
    complete_ = false;
  }
}

Dx12ImageRetentionSample Dx12ImageRetention::sample() noexcept {
  prune();
  if (!complete_) return {.byte_count = std::nullopt};
  Dx12ImageRetentionSample result;
  for (const auto& observed : images_) {
    // Keep only this observation alive, not a vector of new strong references.
    if (const auto image = observed.lock()) dx12_add_retained_image_bytes(result, image->pixels.size());
  }
  return result;
}
} // namespace cgpui
