#pragma once

#include "dx12_native_internal.hpp"
#include <cstddef>
#include <memory>
#include <vector>

namespace cgpui {
struct Dx12GeometryUploadSlice {
  ID3D12Resource* resource = nullptr;
  D3D12_GPU_VIRTUAL_ADDRESS address = 0;
  std::size_t offset = 0;
  UINT size = 0;
};

struct Dx12GeometryUploadStats {
  std::size_t page_count = 0;
  std::size_t capacity_bytes = 0;
  // Includes alignment between slices, but not unused page tails.
  std::size_t used_bytes = 0;
  std::size_t allocations_this_frame = 0;
  std::size_t total_allocations = 0;
};

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
  struct Page;
  Result<Page*> create_page(ID3D12Device& device, std::size_t capacity, bool transient);
  std::vector<std::unique_ptr<Page>> pages_;
  std::size_t page_cursor_ = 0;
  Dx12GeometryUploadStats stats_;
};
} // namespace cgpui
