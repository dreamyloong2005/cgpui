#pragma once

#include "dx12_srv_topology_test_support.hpp"
#include "../../src/renderer/dx12/dx12_srv_descriptor_pool_internal.hpp"
#include "../../src/renderer/dx12/dx12_srv_heap_api_internal.hpp"

namespace cgpui_test {
class SrvFailingHeapApi final : public cgpui::Dx12SrvHeapApi {
 public:
  HRESULT create(ID3D12Device& device, const D3D12_DESCRIPTOR_HEAP_DESC& descriptor,
                 ID3D12DescriptorHeap** heap) noexcept override;
  unsigned attempts = 0;
  unsigned successes = 0;
  unsigned fail_on = 0;
  HRESULT failure = E_ACCESSDENIED;
};
bool srv_heap_sentinel(ID3D12DescriptorHeap& heap, std::atomic_bool& released);
} // namespace cgpui_test
