#pragma once

#include "dx12_texture_internal.hpp"
#include <cstddef>
#include <optional>
#include <vector>

namespace cgpui {
struct Dx12ImageRetentionSample {
  std::optional<std::size_t> byte_count = std::size_t{0};
  bool saturated = false;
};
void dx12_add_retained_image_bytes(Dx12ImageRetentionSample& sample, std::size_t bytes) noexcept;

// Image-only CPU ownership census, not a cache-residency or native-memory counter.
// Callers serialize registration/sampling with the renderer's operation gate.
class Dx12ImageRetention {
 public:
  void track(const Dx12TextureHandle& image) noexcept;
  Dx12ImageRetentionSample sample() noexcept;

 private:
  void prune() noexcept;
  std::vector<std::weak_ptr<const Dx12Texture>> images_;
  // A missed registration cannot become known again merely by pruning entries:
  // the unobserved snapshot might still live in a finished frame.
  bool complete_ = true;
};
} // namespace cgpui
