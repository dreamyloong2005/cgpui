#include "dx12_texture_internal.hpp"
#include <cstring>

namespace cgpui {
Result<Dx12TextureHandle> dx12_create_texture(
    Dx12State&, std::uint32_t width, std::uint32_t height,
    std::uint32_t stride, DXGI_FORMAT format,
    std::span<const std::uint8_t> pixels) {
  const UINT channels = format == DXGI_FORMAT_R8_UNORM ? 1 :
      format == DXGI_FORMAT_R8G8B8A8_UNORM ? 4 : 0;
  const auto row_bytes = std::uint64_t(width) * channels;
  const auto required = height ? std::uint64_t(stride) * (height - 1) + row_bytes : 0;
  if (!channels || !width || !height || width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
      height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION || stride < row_bytes ||
      pixels.size() < required) {
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 texture requires supported format, valid extent, stride and pixel bytes"});
  }
  auto texture = std::make_shared<Dx12Texture>();
  texture->width = width;
  texture->height = height;
  texture->stride = static_cast<UINT>(row_bytes);
  texture->format = format;
  texture->pixels.resize(static_cast<std::size_t>(row_bytes * height));
  for (UINT row = 0; row < height; ++row) {
    std::memcpy(texture->pixels.data() + row * row_bytes,
                pixels.data() + std::size_t(row) * stride, static_cast<std::size_t>(row_bytes));
  }
  return Dx12TextureHandle{std::move(texture)};
}
} // namespace cgpui
