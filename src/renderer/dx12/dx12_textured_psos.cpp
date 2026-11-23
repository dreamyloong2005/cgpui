#include "dx12_texture_renderer_internal.hpp"
#include "dx12_draw_commands_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_shader_binaries_internal.hpp"
#include "dx12_pipeline_state_internal.hpp"
#include <cstddef>

namespace cgpui {
Result<void> dx12_create_textured_psos(Dx12State& state) {
  auto& renderer = *state.texture_renderer;
  const D3D12_INPUT_ELEMENT_DESC inputs[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Dx12TextureVertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Dx12TextureVertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Dx12TextureVertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
  auto pipeline = dx12_pipeline_defaults(renderer.root_signature.Get());
  pipeline.InputLayout = {inputs, 3};
  pipeline.VS = dx12_image_vertex_shader();
  pipeline.PS = dx12_image_fragment_shader();
  HRESULT result = state.device->CreateGraphicsPipelineState(&pipeline,
      IID_PPV_ARGS(&renderer.image_pipeline));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "image pipeline"));
  pipeline.VS = dx12_text_vertex_shader();
  pipeline.PS = dx12_text_fragment_shader();
  result = state.device->CreateGraphicsPipelineState(&pipeline,
      IID_PPV_ARGS(&renderer.text_pipeline));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "text pipeline"));
  return dx12_check_debug_errors(state);
}
} // namespace cgpui
