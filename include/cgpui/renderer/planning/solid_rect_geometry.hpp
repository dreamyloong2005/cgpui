#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "cgpui/renderer/planning/rect_geometry_types.hpp"

#include <span>

namespace cgpui {

[[nodiscard]] RectGeometry build_solid_rect_geometry(std::span<const SolidRect> rects);

} // namespace cgpui
