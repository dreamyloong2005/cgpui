#include "dx12_texture_renderer_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Result<void> dx12_create_texture_pipeline(Dx12State& state) {
  auto& renderer = *state.texture_renderer;
  if (renderer.image_pipeline && renderer.text_pipeline) return {};
  D3D12_DESCRIPTOR_RANGE ranges[2]{};
  ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  ranges[0].NumDescriptors = 1;
  ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
  ranges[1].NumDescriptors = 1;
  D3D12_ROOT_PARAMETER parameters[3]{};
  parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
  parameters[0].Constants.Num32BitValues = 2;
  parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  for (UINT index = 0; index < 2; ++index) {
    parameters[index + 1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    parameters[index + 1].DescriptorTable = {1, &ranges[index]};
    parameters[index + 1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  }
  D3D12_ROOT_SIGNATURE_DESC signature{};
  signature.NumParameters = 3;
  signature.pParameters = parameters;
  signature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
  Dx12Ptr<ID3DBlob> blob, errors;
  HRESULT result = D3D12SerializeRootSignature(&signature, D3D_ROOT_SIGNATURE_VERSION_1,
      &blob, &errors);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "texture root signature serialization"));
  result = state.device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
      IID_PPV_ARGS(&renderer.root_signature));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "texture root signature creation"));
  D3D12_DESCRIPTOR_HEAP_DESC descriptor{};
  descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
  descriptor.NumDescriptors = 2;
  descriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  result = state.device->CreateDescriptorHeap(&descriptor, IID_PPV_ARGS(&renderer.samplers));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "texture sampler heap"));
  renderer.sampler_stride = state.device->GetDescriptorHandleIncrementSize(descriptor.Type);
  auto handle = renderer.samplers->GetCPUDescriptorHandleForHeapStart();
  for (UINT index = 0; index < 2; ++index) {
    D3D12_SAMPLER_DESC sampler{};
    sampler.Filter = index == 0 ? D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler.MaxAnisotropy = 1;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    state.device->CreateSampler(&sampler, handle);
    handle.ptr += renderer.sampler_stride;
  }
  return dx12_create_textured_psos(state);
}
} // namespace cgpui
