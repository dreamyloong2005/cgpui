#pragma once

#include "cgpui/renderer/renderer_effect_draws.hpp"
#include "cgpui/renderer/planning/box_shadow_vertex.hpp"
#include "cgpui/renderer/planning/rect_geometry_types.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace cgpui {

struct BoxShadowGeometry {
  std::vector<BoxShadowVertex> vertices;
  std::vector<std::uint32_t> indices;
  std::vector<RectDrawRange> draws;
};

// Input lengths are already in device pixels. The quad covers the shadow
// shape grown by 3 sigma plus one antialiasing pixel.
[[nodiscard]] BoxShadowGeometry build_box_shadow_geometry(
    std::span<const BoxShadowDraw> shadows);

} // namespace cgpui
