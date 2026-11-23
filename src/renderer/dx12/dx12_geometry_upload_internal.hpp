#pragma once

#include "dx12_upload_pages_internal.hpp"

namespace cgpui {
struct Dx12GeometryUploadSlice {
  ID3D12Resource* resource = nullptr;
  D3D12_GPU_VIRTUAL_ADDRESS address = 0;
  std::size_t offset = 0;
  UINT size = 0;
};

using Dx12GeometryUploadStats = Dx12UploadPageStats;

// Geometry only: texture footprints and readback retain their own owners.
// Slices borrow pages until begin_frame/clear. The caller must prove completion
// before rewinding, and must clear all pages before retiring their native device.
class Dx12GeometryUploads {
 public:
  Dx12GeometryUploads();
  ~Dx12GeometryUploads();
  Dx12GeometryUploads(const Dx12GeometryUploads&) = delete;
  Dx12GeometryUploads& operator=(const Dx12GeometryUploads&) = delete;
  Result<Dx12GeometryUploadSlice> upload(
      ID3D12Device& device, const void* data, std::size_t size);
  void begin_frame();
  void clear() noexcept;
  [[nodiscard]] Dx12GeometryUploadStats stats() const noexcept;

 private:
  Dx12UploadPages pages_;
};
} // namespace cgpui
