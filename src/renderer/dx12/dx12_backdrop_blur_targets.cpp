#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Result<std::unique_ptr<Dx12BackdropBlurTargets>> dx12_create_backdrop_blur_targets(
    Dx12State& state, PixelExtent texels) {
  auto resources = std::make_unique<Dx12BackdropBlurTargets>();
  auto srvs = state.srv_pool->allocate(*state.device.Get(), 3);
  if (!srvs) return std::unexpected(srvs.error());
  resources->srvs = std::move(*srvs);
  D3D12_DESCRIPTOR_HEAP_DESC heap{};
  heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  heap.NumDescriptors = 2;
  heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  HRESULT result = state.device->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&resources->rtvs));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "backdrop RTV heap"));
  const UINT rtv_stride = state.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  auto rtv = resources->rtvs->GetCPUDescriptorHandleForHeapStart();
  D3D12_HEAP_PROPERTIES properties{};
  properties.Type = D3D12_HEAP_TYPE_DEFAULT;
  for (std::size_t index = 0; index < resources->targets.size(); ++index) {
    auto& target = resources->targets[index];
    D3D12_RESOURCE_DESC descriptor{};
    descriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    descriptor.Width = index == 0 ? state.width : texels.width;
    descriptor.Height = index == 0 ? state.height : texels.height;
    descriptor.DepthOrArraySize = 1;
    descriptor.MipLevels = 1;
    descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    descriptor.SampleDesc.Count = 1;
    descriptor.Flags = index == 0 ? D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    const auto initial = index == 0 ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_RENDER_TARGET;
    result = state.device->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE,
        &descriptor, initial, nullptr, IID_PPV_ARGS(&target.resource));
    if (FAILED(result)) return std::unexpected(dx12_error(result, "backdrop target allocation"));
    D3D12_SHADER_RESOURCE_VIEW_DESC view{};
    view.Format = descriptor.Format;
    view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    view.Texture2D.MipLevels = 1;
    state.device->CreateShaderResourceView(target.resource.Get(), &view,
        resources->srvs.cpu(static_cast<unsigned>(index)));
    target.srv = resources->srvs.gpu(static_cast<unsigned>(index));
    if (index != 0) {
      state.device->CreateRenderTargetView(target.resource.Get(), nullptr, rtv);
      target.rtv = rtv;
      rtv.ptr += rtv_stride;
    }
  }
  return resources;
}
} // namespace cgpui
