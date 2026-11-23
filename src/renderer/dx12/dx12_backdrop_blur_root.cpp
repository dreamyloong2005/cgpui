#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Result<void> dx12_create_backdrop_blur_root(Dx12State& state) {
  auto& renderer = *state.backdrop_blur;
  if (renderer.root_signature) return {};
  D3D12_DESCRIPTOR_RANGE range{};
  range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  range.NumDescriptors = 1;
  D3D12_DESCRIPTOR_RANGE scene_range = range;
  scene_range.BaseShaderRegister = 1;
  D3D12_ROOT_PARAMETER parameters[3]{};
  parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
  parameters[0].Constants.Num32BitValues = sizeof(Dx12BackdropBlurConstants) / sizeof(UINT);
  parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
  parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  parameters[1].DescriptorTable = {1, &range};
  parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  parameters[2].DescriptorTable = {1, &scene_range};
  parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  D3D12_STATIC_SAMPLER_DESC sampler{};
  sampler.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
  sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
  sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
  sampler.MaxAnisotropy = 1;
  sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  D3D12_ROOT_SIGNATURE_DESC signature{};
  signature.NumParameters = state.composition_surface ? 3 : 2;
  signature.pParameters = parameters;
  signature.NumStaticSamplers = 1;
  signature.pStaticSamplers = &sampler;
  signature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
  Dx12Ptr<ID3DBlob> blob, errors;
  HRESULT result = D3D12SerializeRootSignature(&signature, D3D_ROOT_SIGNATURE_VERSION_1,
      &blob, &errors);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "backdrop root signature serialization"));
  result = state.device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
      IID_PPV_ARGS(&renderer.root_signature));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "backdrop root signature creation"));
  return {};
}
} // namespace cgpui
