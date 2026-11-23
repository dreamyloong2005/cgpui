#include "dx12_geometry_upload_internal.hpp"
#include "dx12_buffers_internal.hpp"
#include <algorithm>
#include <cstring>
#include <limits>

namespace cgpui {
namespace {
constexpr std::size_t page_bytes = 256 * 1024;
constexpr std::size_t retained_pages = 4;
constexpr std::size_t slice_alignment = 16;
constexpr std::size_t large_page_alignment = 64 * 1024;

Error invalid_upload() {
  return {.code = ErrorCode::invalid_argument,
      .message = "DX12 geometry upload requires nonempty data and representable aligned sizes"};
}

bool align_size(std::size_t value, std::size_t alignment, std::size_t& aligned) {
  const auto padding = (alignment - value % alignment) % alignment;
  if (padding > std::numeric_limits<std::size_t>::max() - value) return false;
  aligned = value + padding;
  return true;
}

void increment(std::size_t& count) {
  if (count != std::numeric_limits<std::size_t>::max()) ++count;
}
} // namespace

struct Dx12GeometryUploads::Page {
  Dx12Ptr<ID3D12Resource> resource;
  std::byte* mapped = nullptr;
  std::size_t capacity = 0;
  std::size_t used = 0;
  bool transient = false;

  ~Page() {
    // Persistent mapping is balanced only when the page actually retires.
    // nullptr permits the whole written range across every reuse of this page.
    if (mapped) resource->Unmap(0, nullptr);
  }
};

Dx12GeometryUploads::Dx12GeometryUploads() = default;
Dx12GeometryUploads::~Dx12GeometryUploads() = default;

Result<Dx12GeometryUploads::Page*> Dx12GeometryUploads::create_page(
    ID3D12Device& device, std::size_t capacity, bool transient) {
  if (capacity > std::numeric_limits<std::size_t>::max() - stats_.capacity_bytes)
    return std::unexpected(invalid_upload());
  auto page = std::make_unique<Page>();
  auto resource = dx12_create_buffer(device, capacity, D3D12_HEAP_TYPE_UPLOAD);
  if (!resource) return std::unexpected(resource.error());
  void* mapped = nullptr;
  const D3D12_RANGE read{0, 0};
  const HRESULT result = (*resource)->Map(0, &read, &mapped);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "geometry upload page mapping"));
  page->resource = std::move(*resource);
  page->mapped = static_cast<std::byte*>(mapped);
  page->capacity = capacity;
  page->transient = transient;
  auto* created = page.get();
  pages_.push_back(std::move(page));
  stats_.page_count = pages_.size();
  stats_.capacity_bytes += capacity;
  increment(stats_.allocations_this_frame);
  increment(stats_.total_allocations);
  return created;
}

Result<Dx12GeometryUploadSlice> Dx12GeometryUploads::upload(
    ID3D12Device& device, const void* data, std::size_t size) {
  if (!data || !size || size > std::numeric_limits<UINT>::max())
    return std::unexpected(invalid_upload());
  Page* page = nullptr;
  std::size_t offset = 0;
  if (size <= page_bytes) {
    // Monotonic within a frame: even many tiny draws visit each full page once.
    for (; page_cursor_ < pages_.size(); ++page_cursor_) {
      auto& candidate = *pages_[page_cursor_];
      if (candidate.transient) continue;
      if (!align_size(candidate.used, slice_alignment, offset))
        return std::unexpected(invalid_upload());
      if (offset <= candidate.capacity && size <= candidate.capacity - offset) {
        page = &candidate;
        break;
      }
    }
  }
  if (!page) {
    const bool transient = size > page_bytes;
    std::size_t capacity = page_bytes;
    if (transient && !align_size(size, large_page_alignment, capacity))
      return std::unexpected(invalid_upload());
    auto created = create_page(device, capacity, transient);
    if (!created) return std::unexpected(created.error());
    page = *created;
    offset = 0;
    if (!transient) page_cursor_ = pages_.size() - 1;
  }
  std::memcpy(page->mapped + offset, data, size);
  const auto used = offset + size; // Bounded by the selected page's capacity.
  stats_.used_bytes += used - page->used;
  page->used = used;
  return Dx12GeometryUploadSlice{.resource = page->resource.Get(),
      .address = page->resource->GetGPUVirtualAddress() + offset,
      .offset = offset, .size = static_cast<UINT>(size)};
}

void Dx12GeometryUploads::begin_frame() {
  std::size_t retained = 0;
  std::erase_if(pages_, [&](const auto& page) {
    if (page->transient || retained >= retained_pages) return true;
    ++retained;
    page->used = 0;
    return false;
  });
  page_cursor_ = 0;
  stats_.page_count = pages_.size();
  stats_.capacity_bytes = pages_.size() * page_bytes;
  stats_.used_bytes = 0;
  stats_.allocations_this_frame = 0;
}

void Dx12GeometryUploads::clear() noexcept {
  pages_.clear();
  page_cursor_ = 0;
  const auto total = stats_.total_allocations;
  stats_ = {};
  stats_.total_allocations = total;
}

Dx12GeometryUploadStats Dx12GeometryUploads::stats() const noexcept { return stats_; }
} // namespace cgpui
