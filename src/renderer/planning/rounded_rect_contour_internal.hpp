#pragma once

#include "cgpui/core/paint_values.hpp"
#include "cgpui/renderer/planning/rect_vertex.hpp"

#include <cstddef>
#include <vector>

namespace cgpui {

[[nodiscard]] std::size_t rounded_rect_perimeter_vertex_count(
    std::size_t corner_segment_count);

void append_rounded_rect_vertex(
    std::vector<RectVertex>& vertices,
    Point point,
    Color color,
    float coverage);

void append_rounded_rect_contour(
    std::vector<RectVertex>& vertices,
    Rect rect,
    BorderRadii radii,
    std::size_t corner_segment_count,
    Color color,
    float coverage);

} // namespace cgpui
