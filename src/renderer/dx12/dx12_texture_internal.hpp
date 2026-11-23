#pragma once

#include "dx12_native_internal.hpp"
#include <memory>
#include <span>
#include <vector>

namespace cgpui {
struct Dx12State;

// Draws own immutable snapshots. Cache replacement/invalidation cannot change
// the bytes referenced by an earlier draw, including within the same frame.
struct Dx12Texture {
  UINT width = 0, height = 0, stride = 0;
  DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
  std::vector<std::uint8_t> pixels;
  mutable Dx12Ptr<ID3D12Resource> resource;
  mutable Dx12Ptr<ID3D12DescriptorHeap> descriptors;
  mutable bool uploaded = false;
};
using Dx12TextureHandle = std::shared_ptr<const Dx12Texture>;

Result<Dx12TextureHandle> dx12_create_texture(
    Dx12State& state, std::uint32_t width, std::uint32_t height,
    std::uint32_t stride, DXGI_FORMAT format,
    std::span<const std::uint8_t> pixels);
Result<void> dx12_prepare_texture(Dx12State& state, const Dx12TextureHandle& texture);
} // namespace cgpui
