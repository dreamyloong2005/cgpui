#include "dx12_draw_commands_internal.hpp"
#include "dx12_prepared_draw_internal.hpp"
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
  const auto realization = dx12_prepare_texture(state, draw.texture, draw.glyph);
  if (!realization) return std::unexpected(realization.error());
  const auto& texture = **realization;
  const auto byte_size = draw.vertices.size() * sizeof(Dx12TextureVertex);
  auto vertices = dx12_upload_geometry(state, draw.vertices.data(), byte_size);
  if (!vertices) return std::unexpected(vertices.error());
  auto& renderer = *state.texture_renderer;
  auto sampler = renderer.samplers->GetGPUDescriptorHandleForHeapStart();
  if (draw.sampling == ImageSamplingMode::linear) sampler.ptr += renderer.sampler_stride;
  const Dx12PreparedDraw prepared{
      .pipeline = draw.glyph ? renderer.text_pipeline.Get() : renderer.image_pipeline.Get(),
      .root = renderer.root_signature.Get(), .extent = {float(state.width), float(state.height)},
      .vertex = {vertices->address, vertices->size, sizeof(Dx12TextureVertex)},
      .srv_heap = texture.srv.heap(), .sampler_heap = renderer.samplers.Get(),
      .srv = texture.srv.gpu(), .sampler = sampler, .count = static_cast<UINT>(draw.vertices.size())};
  const auto clip = clipped.scissor;
  const D3D12_RECT scissor{clip.x, clip.y, static_cast<LONG>(clip.x + clip.width),
      static_cast<LONG>(clip.y + clip.height)};
  state.commands->RSSetScissorRects(1, &scissor);
  dx12_emit_prepared_draw(state, *state.commands.Get(), prepared);
  if (state.active_diagnostics) state.active_diagnostics->submitted();
  return {};
}
} // namespace cgpui
