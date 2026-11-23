#include "dx12_texture_renderer_internal.hpp"
#include "dx12_draw_commands_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_shader_binaries_internal.hpp"
#include <cstddef>

namespace cgpui {
Result<void> dx12_create_textured_psos(Dx12State& state) {
  auto& renderer = *state.texture_renderer;
  const D3D12_INPUT_ELEMENT_DESC inputs[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Dx12TextureVertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Dx12TextureVertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Dx12TextureVertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
  D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline{};
  pipeline.pRootSignature = renderer.root_signature.Get();
  pipeline.InputLayout = {inputs, 3};
  pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pipeline.NumRenderTargets = 1;
  pipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  pipeline.SampleDesc.Count = 1;
  pipeline.SampleMask = UINT_MAX;
  pipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
  pipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
  pipeline.RasterizerState.DepthClipEnable = TRUE;
  pipeline.DepthStencilState.DepthEnable = FALSE;
  pipeline.DepthStencilState.StencilEnable = FALSE;
  auto& blend = pipeline.BlendState.RenderTarget[0];
  blend.BlendEnable = TRUE;
  blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
  blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
  blend.BlendOp = D3D12_BLEND_OP_ADD;
  blend.SrcBlendAlpha = D3D12_BLEND_ONE;
  blend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
  blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
  blend.LogicOp = D3D12_LOGIC_OP_NOOP;
  blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
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
