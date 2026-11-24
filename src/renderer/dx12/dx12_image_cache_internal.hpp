#pragma once

#include "dx12_texture_internal.hpp"
#include "dx12_image_retention_internal.hpp"
#include <unordered_map>

namespace cgpui {
class Dx12ImageCache {
 public:
  Result<void> upload(Dx12State& state, const ImageAsset& image);
  void invalidate(ImageAssetId id);
  Result<Dx12TextureHandle> find(const ImageAssetDescriptor& descriptor);
  void finish_frame() noexcept;
  Dx12ImageRetentionSample retained_image_payload() noexcept;

 private:
  struct Entry {
    ImageAssetDescriptor descriptor;
    Dx12TextureHandle texture;
    unsigned idle_frames = 0;
    bool touched = true;
  };
  std::unordered_map<std::uint64_t, Entry> entries_;
  Dx12ImageRetention retention_;
};
} // namespace cgpui
