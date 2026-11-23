#include "dx12_backdrop_blur_internal.hpp"

namespace cgpui {
RendererEffectSupport dx12_backdrop_blur_support(ID3D12Device& device) {
  D3D12_FEATURE_DATA_FORMAT_SUPPORT format{DXGI_FORMAT_R8G8B8A8_UNORM};
  const HRESULT result = device.CheckFeatureSupport(
      D3D12_FEATURE_FORMAT_SUPPORT, &format, sizeof(format));
  if (FAILED(result)) return {.supported = false,
      .reason = dx12_error(result, "backdrop format capability query").message};
  constexpr auto required = D3D12_FORMAT_SUPPORT1_TEXTURE2D |
      D3D12_FORMAT_SUPPORT1_SHADER_LOAD | D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE |
      D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
  if ((format.Support1 & required) != required) return {.supported = false,
      .reason = "DX12 RGBA8 backdrop targets require render, shader-load and filtered-sample support"};
  return {.supported = true};
}
} // namespace cgpui
