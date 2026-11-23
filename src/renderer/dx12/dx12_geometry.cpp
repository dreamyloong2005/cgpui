#include "dx12_frame_internal.hpp"
#include "dx12_buffers_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"
#include <cmath>
#include <limits>

namespace cgpui {
bool dx12_finite_geometry(const RectGeometry& geometry) {
  if (geometry.vertices.size() > UINT_MAX / sizeof(RectVertex) ||
      geometry.indices.size() > UINT_MAX / sizeof(std::uint32_t)) return false;
  for (const auto& vertex : geometry.vertices) {
    for (float value : vertex.position) if (!std::isfinite(value)) return false;
    for (float value : vertex.color) if (!std::isfinite(value)) return false;
    if (!std::isfinite(vertex.coverage)) return false;
  }
  for (const auto index : geometry.indices) if (index >= geometry.vertices.size()) return false;
  for (const auto& draw : geometry.draws) {
    if (draw.first_index > geometry.indices.size() ||
        draw.index_count > geometry.indices.size() - draw.first_index) return false;
    if (draw.clip_rect) {
      const auto& clip = *draw.clip_rect;
      if (!std::isfinite(clip.origin.x) || !std::isfinite(clip.origin.y) ||
          !std::isfinite(clip.size.width) || !std::isfinite(clip.size.height)) return false;
    }
  }
  return true;
}

Result<void> dx12_record_geometry(Dx12State& state, const RectGeometry& geometry) {
  if (geometry.vertices.empty() || geometry.indices.empty()) return {};
  const auto vertex_bytes = geometry.vertices.size() * sizeof(RectVertex);
  const auto index_bytes = geometry.indices.size() * sizeof(std::uint32_t);
  auto vertices = dx12_upload_buffer(*state.device.Get(), geometry.vertices.data(), vertex_bytes);
  if (!vertices) return std::unexpected(vertices.error());
  auto indices = dx12_upload_buffer(*state.device.Get(), geometry.indices.data(), index_bytes);
  if (!indices) return std::unexpected(indices.error());
  const D3D12_VERTEX_BUFFER_VIEW vertex_view{
      (*vertices)->GetGPUVirtualAddress(), static_cast<UINT>(vertex_bytes), sizeof(RectVertex)};
  const D3D12_INDEX_BUFFER_VIEW index_view{
      (*indices)->GetGPUVirtualAddress(), static_cast<UINT>(index_bytes), DXGI_FORMAT_R32_UINT};
  state.commands->IASetVertexBuffers(0, 1, &vertex_view);
  state.commands->IASetIndexBuffer(&index_view);
  for (const auto& draw : geometry.draws) {
    const auto clipped = resolve_clip_stack_scissor(
        PixelExtent{state.width, state.height}, draw.clip_rect, {});
    if (!clipped.visible) continue;
    const auto clip = clipped.scissor;
    const D3D12_RECT scissor{clip.x, clip.y,
        static_cast<LONG>(clip.x + clip.width), static_cast<LONG>(clip.y + clip.height)};
    state.commands->RSSetScissorRects(1, &scissor);
    state.commands->DrawIndexedInstanced(static_cast<UINT>(draw.index_count), 1,
        static_cast<UINT>(draw.first_index), 0, 0);
  }
  state.uploads.push_back(std::move(*vertices));
  state.uploads.push_back(std::move(*indices));
  return {};
}
} // namespace cgpui
