#include "dx12_srv_heap_api_internal.hpp"

namespace cgpui {
namespace {
class NativeSrvHeapApi final : public Dx12SrvHeapApi {
 public:
  HRESULT create(ID3D12Device& device, const D3D12_DESCRIPTOR_HEAP_DESC& descriptor,
                 ID3D12DescriptorHeap** heap) noexcept override {
    return device.CreateDescriptorHeap(&descriptor, IID_PPV_ARGS(heap));
  }
};
} // namespace
Dx12SrvHeapApi& dx12_native_srv_heap_api() noexcept {
  static NativeSrvHeapApi api;
  return api;
}
} // namespace cgpui
