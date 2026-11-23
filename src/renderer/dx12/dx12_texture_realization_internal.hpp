#pragma once

#include "dx12_texture_internal.hpp"

namespace cgpui {
// Weak ownership preserves snapshot identity without pinning invalidated CPU
// images indefinitely. The renderer owns all device-specific realizations.
struct Dx12NativeTexture {
  std::weak_ptr<const Dx12Texture> source;
  Dx12Ptr<ID3D12Resource> resource;
  Dx12Ptr<ID3D12DescriptorHeap> descriptors;
  bool uploaded = false;
};
} // namespace cgpui
