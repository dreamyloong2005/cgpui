#include "dx12_shadow_internal.hpp"
#include <cmath>

namespace cgpui {
namespace {
template <std::size_t Size>
bool finite(const std::array<float, Size>& values) {
  for (float value : values) if (!std::isfinite(value)) return false;
  return true;
}
} // namespace

bool dx12_finite_shadow_geometry(const BoxShadowGeometry& geometry) {
  if (geometry.vertices.size() > UINT_MAX / sizeof(BoxShadowVertex) ||
      geometry.indices.size() > UINT_MAX / sizeof(std::uint32_t)) return false;
  for (const auto& vertex : geometry.vertices) {
    if (!finite(vertex.position) || !finite(vertex.local) || !finite(vertex.color) ||
        !finite(vertex.shadow_rect) || !finite(vertex.shadow_radii) ||
        !finite(vertex.box_rect) || !finite(vertex.box_radii) ||
        !std::isfinite(vertex.sigma) || vertex.sigma < 0) return false;
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
} // namespace cgpui
