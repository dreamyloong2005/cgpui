#pragma once

#include "dx12_native_internal.hpp"
#include <cstddef>
#include <memory>
#include <vector>

namespace cgpui {
struct Dx12UploadPageSlice {
  ID3D12Resource* resource = nullptr;
  std::byte* mapped = nullptr;
  std::size_t offset = 0;
  std::size_t size = 0;
};

struct Dx12UploadPageStats {
  std::size_t page_count = 0;
  std::size_t capacity_bytes = 0;
  // Includes inter-slice alignment, but not unused page tails.
  std::size_t used_bytes = 0;
  std::size_t allocations_this_frame = 0;
  std::size_t total_allocations = 0;
};

// One instance owns one independent pool. Callers prove completion before
// begin_frame, and clear before retiring the device. Returned slices borrow
// pages; mapped points at the slice, not at the resource's first byte.
class Dx12UploadPages {
 public:
  Dx12UploadPages();
  ~Dx12UploadPages();
  Dx12UploadPages(const Dx12UploadPages&) = delete;
  Dx12UploadPages& operator=(const Dx12UploadPages&) = delete;
  Result<Dx12UploadPageSlice> allocate(
      ID3D12Device& device, std::size_t size, std::size_t alignment);
  void begin_frame();
  void clear() noexcept;
  [[nodiscard]] Dx12UploadPageStats stats() const noexcept;

 private:
  struct Page;
  Result<Page*> create_page(ID3D12Device& device, std::size_t capacity, bool transient);
  std::vector<std::unique_ptr<Page>> pages_;
  std::size_t page_cursor_ = 0;
  Dx12UploadPageStats stats_;
};
} // namespace cgpui
