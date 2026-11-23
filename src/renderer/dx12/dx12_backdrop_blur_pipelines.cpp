#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_pipeline_state_internal.hpp"
#include "dx12_shader_binaries_internal.hpp"
#include <cstddef>

namespace cgpui {
Result<void> dx12_create_backdrop_blur_pipelines(Dx12State& state) {
  auto& renderer = *state.backdrop_blur;
  if (renderer.downsample_pipeline && renderer.blur_pipeline && renderer.composite_pipeline &&
      (!state.composition_surface || renderer.alpha_composite_pipeline)) return {};
  if (auto root = dx12_create_backdrop_blur_root(state); !root) return root;
  auto pipeline = dx12_pipeline_defaults(renderer.root_signature.Get());
  pipeline.BlendState.RenderTarget[0].BlendEnable = FALSE;
  pipeline.VS = dx12_backdrop_fullscreen_vertex_shader();
  pipeline.PS = dx12_backdrop_downsample_fragment_shader();
  HRESULT result = state.device->CreateGraphicsPipelineState(&pipeline,
      IID_PPV_ARGS(&renderer.downsample_pipeline));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "backdrop downsample pipeline"));
  pipeline.PS = dx12_backdrop_blur_fragment_shader();
  result = state.device->CreateGraphicsPipelineState(&pipeline, IID_PPV_ARGS(&renderer.blur_pipeline));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "backdrop Gaussian pipeline"));
  const D3D12_INPUT_ELEMENT_DESC inputs[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(BackdropBlurVertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"LOCAL", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(BackdropBlurVertex, local), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"RECT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BackdropBlurVertex, rect), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"RADII", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BackdropBlurVertex, radii), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TINT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BackdropBlurVertex, tint), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"REGION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(BackdropBlurVertex, region), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
  pipeline.BlendState.RenderTarget[0].BlendEnable = TRUE;
  pipeline.VS = dx12_backdrop_composite_vertex_shader();
  pipeline.PS = dx12_backdrop_composite_fragment_shader();
  pipeline.InputLayout = {inputs, 6};
  result = state.device->CreateGraphicsPipelineState(&pipeline, IID_PPV_ARGS(&renderer.composite_pipeline));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "backdrop composite pipeline"));
  if (state.composition_surface) {
    // The shader supplies the complete premultiplied replacement RGBA, including
    // original-scene interpolation at rounded edges and partial effect opacity.
    pipeline.BlendState.RenderTarget[0].BlendEnable = FALSE;
    pipeline.PS = dx12_backdrop_composite_alpha_fragment_shader();
    result = state.device->CreateGraphicsPipelineState(&pipeline,
        IID_PPV_ARGS(&renderer.alpha_composite_pipeline));
    if (FAILED(result)) return std::unexpected(dx12_error(result, "backdrop alpha replacement pipeline"));
  }
  return dx12_check_debug_errors(state);
}
} // namespace cgpui
