#pragma once

#include "dx12_native_internal.hpp"
#include <memory>
#include <span>
#include <vector>

namespace cgpui {
struct Dx12State;

// Draws own immutable CPU snapshots. Native realizations belong to the renderer,
// so an old frame cannot keep a removed device alive through a cached texture.
struct Dx12Texture {
  UINT width = 0, height = 0, stride = 0;
  DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
  std::vector<std::uint8_t> pixels;
};
using Dx12TextureHandle = std::shared_ptr<const Dx12Texture>;

Result<Dx12TextureHandle> dx12_create_texture(
    Dx12State& state, std::uint32_t width, std::uint32_t height,
    std::uint32_t stride, DXGI_FORMAT format,
    std::span<const std::uint8_t> pixels);
} // namespace cgpui
