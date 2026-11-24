#include "dx12_draw_commands_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_geometry_upload_frame_internal.hpp"
#include "dx12_frame_diagnostics_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"
#include <cmath>

namespace cgpui {
bool dx12_finite_textured_draw(const Dx12TexturedDraw& draw) {
  if (!draw.texture || draw.vertices.size() > UINT_MAX / sizeof(Dx12TextureVertex) ||
      draw.vertices.size() % 3 != 0 ||
      (draw.sampling != ImageSamplingMode::nearest && draw.sampling != ImageSamplingMode::linear) ||
      draw.texture->format != (draw.glyph ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM)) return false;
  for (const auto& vertex : draw.vertices) {
    for (float value : vertex.position) if (!std::isfinite(value)) return false;
    for (float value : vertex.uv) if (!std::isfinite(value)) return false;
    for (float value : vertex.color) if (!std::isfinite(value)) return false;
  }
  if (draw.clip_rect) {
    const auto& clip = *draw.clip_rect;
    if (!std::isfinite(clip.origin.x) || !std::isfinite(clip.origin.y) ||
        !std::isfinite(clip.size.width) || !std::isfinite(clip.size.height)) return false;
  }
  return true;
}

Result<void> dx12_record_textured_draw(Dx12State& state, const Dx12TexturedDraw& draw) {
  if (draw.vertices.empty()) return {};
  const auto clipped = resolve_clip_stack_scissor(
      PixelExtent{state.width, state.height}, draw.clip_rect, {});
  if (!clipped.visible) return {};
  if (auto pipeline = dx12_create_texture_pipeline(state); !pipeline) return pipeline;
  const auto prepared = dx12_prepare_texture(state, draw.texture);
  if (!prepared) return std::unexpected(prepared.error());
  const auto& texture = **prepared;
  const auto byte_size = draw.vertices.size() * sizeof(Dx12TextureVertex);
  auto vertices = dx12_upload_geometry(state, draw.vertices.data(), byte_size);
  if (!vertices) return std::unexpected(vertices.error());
  auto& renderer = *state.texture_renderer;
  state.commands->SetPipelineState(draw.glyph ? renderer.text_pipeline.Get() : renderer.image_pipeline.Get());
  state.commands->SetGraphicsRootSignature(renderer.root_signature.Get());
  const float extent[] = {static_cast<float>(state.width), static_cast<float>(state.height)};
  state.commands->SetGraphicsRoot32BitConstants(0, 2, extent, 0);
  ID3D12DescriptorHeap* heaps[] = {texture.srv.heap(), renderer.samplers.Get()};
  state.commands->SetDescriptorHeaps(2, heaps);
  state.commands->SetGraphicsRootDescriptorTable(1,
      texture.srv.gpu());
  auto sampler = renderer.samplers->GetGPUDescriptorHandleForHeapStart();
  if (draw.sampling == ImageSamplingMode::linear) sampler.ptr += renderer.sampler_stride;
  state.commands->SetGraphicsRootDescriptorTable(2, sampler);
  const D3D12_VERTEX_BUFFER_VIEW view{vertices->address, vertices->size, sizeof(Dx12TextureVertex)};
  state.commands->IASetVertexBuffers(0, 1, &view);
  const auto clip = clipped.scissor;
  const D3D12_RECT scissor{clip.x, clip.y, static_cast<LONG>(clip.x + clip.width),
      static_cast<LONG>(clip.y + clip.height)};
  state.commands->RSSetScissorRects(1, &scissor);
  state.commands->DrawInstanced(static_cast<UINT>(draw.vertices.size()), 1, 0, 0);
  if (state.active_diagnostics) state.active_diagnostics->submitted();
  return {};
}
} // namespace cgpui
