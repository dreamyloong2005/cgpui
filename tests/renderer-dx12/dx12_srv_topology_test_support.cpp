#include "dx12_srv_topology_test_support.hpp"
#include "../../src/renderer/dx12/dx12_texture_renderer_internal.hpp"
#include "../../src/renderer/dx12/dx12_backdrop_blur_internal.hpp"
#include <set>

namespace cgpui_test {
namespace {
bool observe(ID3D12Device& device, ID3D12DescriptorHeap* heap,
             D3D12_GPU_DESCRIPTOR_HANDLE handle, std::vector<SrvNativeObservation>& records) {
  if (!heap) return false;
  const auto descriptor = heap->GetDesc();
  const UINT stride = device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  const auto start = heap->GetGPUDescriptorHandleForHeapStart().ptr;
  cgpui::Dx12Ptr<IUnknown> canonical;
  if (descriptor.Type != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
      descriptor.Flags != D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ||
      !descriptor.NumDescriptors || !stride || !start || handle.ptr < start ||
      (handle.ptr - start) % stride || (handle.ptr - start) / stride >= descriptor.NumDescriptors ||
      FAILED(heap->QueryInterface(IID_PPV_ARGS(&canonical)))) return false;
  records.push_back({reinterpret_cast<std::uintptr_t>(canonical.Get()), handle.ptr,
                    start, descriptor.NumDescriptors, stride});
  return true;
}
} // namespace
bool srv_native_observations(const cgpui::Dx12State& state,
                             std::vector<SrvNativeObservation>& records) {
  records.clear();
  if (!state.device || state.info.backend != cgpui::RendererBackend::dx12) return false;
  for (const auto& [source, native] : state.texture_renderer->native_textures) {
    if (!source || !native.resource || !native.srv.heap() || !native.uploaded || native.source.expired() ||
        !observe(*state.device.Get(), native.srv.heap(),
                 native.srv.gpu(), records)) return false;
  }
  for (const auto& group : state.backdrop_blur->frame_targets)
    for (const auto& target : group->targets)
      if (!target.resource || !observe(*state.device.Get(), group->srvs.heap(), target.srv, records)) return false;
  return true;
}
bool srv_topology(const cgpui::Dx12State& state, std::size_t views, std::size_t heaps) {
  std::vector<SrvNativeObservation> records;
  if (!srv_native_observations(state, records)) return false;
  std::set<std::uintptr_t> identities;
  std::set<UINT64> handles;
  for (const auto& record : records) { identities.insert(record.heap_identity); handles.insert(record.gpu); }
  std::cout << "SRV topology: views=" << records.size() << " heaps=" << identities.size()
            << " unique_handles=" << handles.size() << " expected_heaps=" << heaps << '\n';
  return records.size() == views && handles.size() == views && identities.size() == heaps;
}
} // namespace cgpui_test
