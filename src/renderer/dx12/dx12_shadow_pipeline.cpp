#include "dx12_shadow_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_pipeline_state_internal.hpp"
#include "dx12_shader_binaries_internal.hpp"
#include <cstddef>

namespace cgpui {
Result<void> dx12_create_shadow_pipeline(Dx12State& state) {
  if (state.shadow_pipeline) return {};
  const D3D12_INPUT_ELEMENT_DESC inputs[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(BoxShadowVertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"LOCAL", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(BoxShadowVertex, local), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BoxShadowVertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"SHADOW_RECT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BoxShadowVertex, shadow_rect), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"SHADOW_RADII", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BoxShadowVertex, shadow_radii), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"BOX_RECT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BoxShadowVertex, box_rect), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"BOX_RADII", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BoxShadowVertex, box_radii), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"SIGMA", 0, DXGI_FORMAT_R32_FLOAT, 0, offsetof(BoxShadowVertex, sigma), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
  auto pipeline = dx12_pipeline_defaults(state.root_signature.Get());
  pipeline.VS = dx12_shadow_vertex_shader();
  pipeline.PS = dx12_shadow_fragment_shader();
  pipeline.InputLayout = {inputs, 8};
  const HRESULT result = state.device->CreateGraphicsPipelineState(&pipeline,
      IID_PPV_ARGS(&state.shadow_pipeline));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "box shadow pipeline"));
  return dx12_check_debug_errors(state);
}
} // namespace cgpui
