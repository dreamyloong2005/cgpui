#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
// Only SRV page creation varies. Sampler/RTV and device policy are unchanged.
class Dx12SrvHeapApi {
 public:
  virtual ~Dx12SrvHeapApi() = default;
  virtual HRESULT create(ID3D12Device& device,
      const D3D12_DESCRIPTOR_HEAP_DESC& descriptor, ID3D12DescriptorHeap** heap) noexcept = 0;
};
Dx12SrvHeapApi& dx12_native_srv_heap_api() noexcept;
} // namespace cgpui
