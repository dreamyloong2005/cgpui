#include "dx12_geometry_upload_internal.hpp"
#include <cstring>
#include <limits>

namespace cgpui {
Dx12GeometryUploads::Dx12GeometryUploads() = default;
Dx12GeometryUploads::~Dx12GeometryUploads() = default;

Result<Dx12GeometryUploadSlice> Dx12GeometryUploads::upload(
    ID3D12Device& device, const void* data, std::size_t size) {
  if (!data || !size || size > std::numeric_limits<UINT>::max())
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 geometry upload requires nonempty data and UINT-representable size"});
  auto slice = pages_.allocate(device, size, 16);
  if (!slice) return std::unexpected(slice.error());
  std::memcpy(slice->mapped, data, size);
  return Dx12GeometryUploadSlice{.resource = slice->resource,
      .address = slice->resource->GetGPUVirtualAddress() + slice->offset,
      .offset = slice->offset, .size = static_cast<UINT>(size)};
}

void Dx12GeometryUploads::begin_frame() { pages_.begin_frame(); }
void Dx12GeometryUploads::clear() noexcept { pages_.clear(); }
Dx12GeometryUploadStats Dx12GeometryUploads::stats() const noexcept { return pages_.stats(); }
} // namespace cgpui
