#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "cgpui/renderer/planning/rect_antialiasing.hpp"
#include "cgpui/renderer/planning/rect_geometry_types.hpp"

#include <span>

namespace cgpui {

[[nodiscard]] RectGeometry build_rounded_rect_geometry(
    std::span<const RoundedRectDraw> rects,
    std::size_t corner_segment_count = 4,
    RectAntialiasingPolicy antialiasing = default_rounded_rect_antialiasing_policy());

} // namespace cgpui
