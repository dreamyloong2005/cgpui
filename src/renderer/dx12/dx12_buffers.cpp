#include "dx12_buffers_internal.hpp"

namespace cgpui {
Result<Dx12Ptr<ID3D12Resource>> dx12_create_buffer(
    ID3D12Device& device, UINT64 size, D3D12_HEAP_TYPE heap) {
  D3D12_HEAP_PROPERTIES properties{};
  properties.Type = heap;
  D3D12_RESOURCE_DESC descriptor{};
  descriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  descriptor.Width = size;
  descriptor.Height = 1;
  descriptor.DepthOrArraySize = 1;
  descriptor.MipLevels = 1;
  descriptor.SampleDesc.Count = 1;
  descriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  Dx12Ptr<ID3D12Resource> resource;
  const auto state = heap == D3D12_HEAP_TYPE_READBACK ? D3D12_RESOURCE_STATE_COPY_DEST :
                                                       D3D12_RESOURCE_STATE_GENERIC_READ;
  const HRESULT result = device.CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE,
      &descriptor, state, nullptr, IID_PPV_ARGS(&resource));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "buffer allocation"));
  return resource;
}

} // namespace cgpui
