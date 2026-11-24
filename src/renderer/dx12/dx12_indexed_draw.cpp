#include "dx12_indexed_draw_internal.hpp"
#include "dx12_prepared_draw_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_geometry_upload_frame_internal.hpp"
#include "dx12_frame_diagnostics_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"

namespace cgpui {
Result<void> dx12_record_indexed_draw(
    Dx12State& state, ID3D12PipelineState& pipeline,
    std::span<const std::byte> vertex_bytes, UINT vertex_stride,
    std::span<const std::uint32_t> index_data, std::span<const RectDrawRange> draws) {
  if (vertex_bytes.empty() || index_data.empty()) return {};
  auto vertices = dx12_upload_geometry(state, vertex_bytes.data(), vertex_bytes.size());
  if (!vertices) return std::unexpected(vertices.error());
  auto indices = dx12_upload_geometry(state, index_data.data(), index_data.size_bytes());
  if (!indices) return std::unexpected(indices.error());
  const D3D12_VERTEX_BUFFER_VIEW vertex_view{
      vertices->address, vertices->size, vertex_stride};
  const D3D12_INDEX_BUFFER_VIEW index_view{
      indices->address, indices->size, DXGI_FORMAT_R32_UINT};
  Dx12PreparedDraw prepared{.pipeline = &pipeline, .root = state.root_signature.Get(),
      .extent = {float(state.width), float(state.height)}, .vertex = vertex_view, .index = index_view, .indexed = true};
  for (std::size_t index = 0; index < draws.size(); ++index) {
    const auto& draw = draws[index];
    const auto clipped = resolve_clip_stack_scissor(
        PixelExtent{state.width, state.height}, draw.clip_rect, {});
    if (!clipped.visible) continue;
    const auto clip = clipped.scissor;
    const D3D12_RECT scissor{clip.x, clip.y,
        static_cast<LONG>(clip.x + clip.width), static_cast<LONG>(clip.y + clip.height)};
    state.commands->RSSetScissorRects(1, &scissor);
    prepared.count = static_cast<UINT>(draw.index_count); prepared.first = static_cast<UINT>(draw.first_index);
    dx12_emit_prepared_draw(state, *state.commands.Get(), prepared);
    if (state.active_diagnostics && draw.index_count) state.active_diagnostics->submitted(index);
  }
  return {};
}
} // namespace cgpui
