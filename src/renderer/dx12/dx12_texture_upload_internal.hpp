#pragma once

#include "dx12_texture_internal.hpp"
#include "dx12_upload_pages_internal.hpp"
#include <span>

namespace cgpui {
struct Dx12TextureSourceView {
  UINT width = 0, height = 0, stride = 0;
  DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
  std::span<const std::uint8_t> pixels;
};
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
  Result<Dx12TextureUploadSlice> upload_region(ID3D12Device& device, const Dx12TextureSourceView& source);
  void begin_frame();
  void clear() noexcept;
  [[nodiscard]] Dx12TextureUploadStats stats() const noexcept;

 private:
  Result<Dx12TextureUploadSlice> upload_view(ID3D12Device&, const Dx12TextureSourceView&);
  Dx12UploadPages pages_;
};
} // namespace cgpui
