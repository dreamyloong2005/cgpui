#include "dx12_frame_internal.hpp"
#include "dx12_indexed_draw_internal.hpp"
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
  return dx12_record_indexed_draw(state, *state.pipeline.Get(),
      std::as_bytes(std::span{geometry.vertices}), sizeof(RectVertex), geometry.indices, geometry.draws);
}
} // namespace cgpui
