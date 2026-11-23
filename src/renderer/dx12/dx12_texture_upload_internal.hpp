#pragma once

#include "dx12_texture_internal.hpp"
#include "dx12_upload_pages_internal.hpp"

namespace cgpui {
struct Dx12TextureUploadSlice {
  ID3D12Resource* resource = nullptr;
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
  std::size_t payload_bytes = 0;
};

using Dx12TextureUploadStats = Dx12UploadPageStats;

// Texture staging has its own pool, separate from geometry. Its footprint
// includes native row/placement padding; payload_bytes excludes that padding.
class Dx12TextureUploads {
 public:
  Dx12TextureUploads();
  ~Dx12TextureUploads();
  Dx12TextureUploads(const Dx12TextureUploads&) = delete;
  Dx12TextureUploads& operator=(const Dx12TextureUploads&) = delete;
  Result<Dx12TextureUploadSlice> upload(ID3D12Device& device, const Dx12Texture& texture);
  void begin_frame();
  void clear() noexcept;
  [[nodiscard]] Dx12TextureUploadStats stats() const noexcept;

 private:
  Dx12UploadPages pages_;
};
} // namespace cgpui
