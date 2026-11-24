#include "dx12_srv_pool_test_support.hpp"

namespace cgpui_test {
HRESULT SrvFailingHeapApi::create(ID3D12Device& device,
    const D3D12_DESCRIPTOR_HEAP_DESC& descriptor, ID3D12DescriptorHeap** heap) noexcept {
  *heap = nullptr;
  if (++attempts == fail_on) return failure;
  const HRESULT result = cgpui::dx12_native_srv_heap_api().create(device, descriptor, heap);
  if (SUCCEEDED(result)) ++successes;
  return result;
}
bool srv_heap_sentinel(ID3D12DescriptorHeap& heap, std::atomic_bool& released) {
  released.store(false);
  cgpui::Dx12Ptr<IUnknown> observer;
  observer.Attach(new (std::nothrow) Dx12ResourceReleaseSentinel(released));
  if (!observer) return false;
  constexpr GUID key{0xe4cb631a, 0x472d, 0x46c7, {0xb4, 0xa8, 0x4e, 0x88, 0x29, 0xa2, 0x00, 0x19}};
  const auto attached = heap.SetPrivateDataInterface(key, observer.Get());
  observer.Reset();
  return SUCCEEDED(attached) && !released.load();
}
} // namespace cgpui_test
