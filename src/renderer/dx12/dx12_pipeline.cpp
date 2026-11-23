#include "dx12_state_internal.hpp"
#include "dx12_shader_binaries_internal.hpp"
#include "dx12_pipeline_state_internal.hpp"
#include "cgpui/renderer/planning/rect_geometry.hpp"
#include <cstddef>

namespace cgpui {
Result<void> dx12_create_pipeline(Dx12State& state) {
  D3D12_ROOT_PARAMETER parameter{};
  parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
  parameter.Constants.ShaderRegister = 0;
  parameter.Constants.Num32BitValues = 2;
  parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  D3D12_ROOT_SIGNATURE_DESC signature{};
  signature.NumParameters = 1;
  signature.pParameters = &parameter;
  signature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
  Dx12Ptr<ID3DBlob> blob, errors;
  HRESULT result = D3D12SerializeRootSignature(&signature, D3D_ROOT_SIGNATURE_VERSION_1,
                                               &blob, &errors);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "root signature serialization"));
  result = state.device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
                                             IID_PPV_ARGS(&state.root_signature));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "root signature creation"));
  const D3D12_INPUT_ELEMENT_DESC inputs[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(RectVertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(RectVertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"COVERAGE", 0, DXGI_FORMAT_R32_FLOAT, 0, offsetof(RectVertex, coverage), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
  auto pipeline = dx12_pipeline_defaults(state.root_signature.Get());
  pipeline.VS = dx12_rect_vertex_shader();
  pipeline.PS = dx12_rect_fragment_shader();
  pipeline.InputLayout = {inputs, 3};
  result = state.device->CreateGraphicsPipelineState(&pipeline, IID_PPV_ARGS(&state.pipeline));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "rectangle pipeline"));
  return dx12_check_debug_errors(state);
}
} // namespace cgpui
