#include "dx12_texture_upload_internal.hpp"
#include <cstring>
#include <limits>

namespace cgpui {
namespace {
Error invalid_texture() {
  return {.code = ErrorCode::invalid_argument,
      .message = "DX12 texture upload requires supported format, bounded extent, stride and pixel bytes"};
}

Result<std::size_t> texture_row_bytes(const Dx12TextureSourceView& texture) {
  const UINT channels = texture.format == DXGI_FORMAT_R8_UNORM ? 1 :
      texture.format == DXGI_FORMAT_R8G8B8A8_UNORM ? 4 : 0;
  if (!channels || !texture.width || !texture.height ||
      texture.width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
      texture.height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION)
    return std::unexpected(invalid_texture());
  const auto row_bytes = std::size_t(texture.width) * channels;
  // The native dimension limit bounds this multiplication even for a padded
  // UINT stride. The last source row needs payload bytes, not trailing padding.
  const auto required = UINT64(texture.stride) * (texture.height - 1) + row_bytes;
  if (texture.stride < row_bytes || required > texture.pixels.size())
    return std::unexpected(invalid_texture());
  return row_bytes;
}

D3D12_RESOURCE_DESC texture_descriptor(const Dx12TextureSourceView& texture) {
  D3D12_RESOURCE_DESC descriptor{};
  descriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  descriptor.Width = texture.width;
  descriptor.Height = texture.height;
  descriptor.DepthOrArraySize = 1;
  descriptor.MipLevels = 1;
  descriptor.Format = texture.format;
  descriptor.SampleDesc.Count = 1;
  return descriptor;
}

bool valid_footprint(const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint,
    UINT rows, UINT64 row_bytes, UINT64 size, const Dx12TextureSourceView& texture, std::size_t expected_row) {
  const auto& layout = footprint.Footprint;
  if (!rows || rows != texture.height || row_bytes != expected_row ||
      layout.Width != texture.width || layout.Height != texture.height || layout.Depth != 1 ||
      layout.Format != texture.format || layout.RowPitch < row_bytes ||
      layout.RowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT ||
      footprint.Offset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT ||
      !size || size == std::numeric_limits<UINT64>::max() ||
      size > std::numeric_limits<std::size_t>::max() || footprint.Offset > size) return false;
  // Division checks the final-row end without overflowing or requiring its
  // trailing RowPitch padding to fit inside the native footprint allocation.
  const auto remaining = size - footprint.Offset;
  return row_bytes <= remaining &&
      UINT64(rows - 1) <= (remaining - row_bytes) / layout.RowPitch;
}
} // namespace

Dx12TextureUploads::Dx12TextureUploads() = default;
Dx12TextureUploads::~Dx12TextureUploads() = default;

Result<Dx12TextureUploadSlice> Dx12TextureUploads::upload_view(
    ID3D12Device& device, const Dx12TextureSourceView& texture) {
  const auto expected_row = texture_row_bytes(texture);
  if (!expected_row) return std::unexpected(expected_row.error());
  const auto descriptor = texture_descriptor(texture);
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
  UINT rows = 0;
  UINT64 row_bytes = 0, size = 0;
  device.GetCopyableFootprints(&descriptor, 0, 1, 0, &footprint, &rows, &row_bytes, &size);
  if (!valid_footprint(footprint, rows, row_bytes, size, texture, *expected_row))
    return std::unexpected(dx12_error(E_INVALIDARG, "texture upload footprint"));
  auto slice = pages_.allocate(device, static_cast<std::size_t>(size),
      D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
  if (!slice) return std::unexpected(slice.error());
  auto* destination = slice->mapped + static_cast<std::size_t>(footprint.Offset);
  for (UINT row = 0; row < rows; ++row) {
    std::memcpy(destination + std::size_t(row) * footprint.Footprint.RowPitch,
        texture.pixels.data() + std::size_t(row) * texture.stride,
        static_cast<std::size_t>(row_bytes));
  }
  // mapped already addresses this slice; only the GPU footprint needs the
  // resource-relative slice offset. Add it exactly once after the CPU copies.
  footprint.Offset += slice->offset;
  return Dx12TextureUploadSlice{.resource = slice->resource, .footprint = footprint,
      .payload_bytes = *expected_row * rows};
}

Result<Dx12TextureUploadSlice> Dx12TextureUploads::upload(
    ID3D12Device& device, const Dx12Texture& texture) {
  return upload_view(device, {texture.width, texture.height, texture.stride, texture.format, texture.pixels});
}
Result<Dx12TextureUploadSlice> Dx12TextureUploads::upload_region(
    ID3D12Device& device, const Dx12TextureSourceView& texture) {
  if (texture.format != DXGI_FORMAT_R8_UNORM) return std::unexpected(invalid_texture());
  return upload_view(device, texture);
}
void Dx12TextureUploads::begin_frame() { pages_.begin_frame(); }
void Dx12TextureUploads::clear() noexcept { pages_.clear(); }
Dx12TextureUploadStats Dx12TextureUploads::stats() const noexcept { return pages_.stats(); }
} // namespace cgpui
