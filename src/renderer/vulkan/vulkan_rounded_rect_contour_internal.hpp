#pragma once

#include "cgpui/ui/style_values.hpp"
#include "vulkan_rounded_rect_vertex_internal.hpp"

#include <cstddef>
#include <vector>

namespace cgpui {

[[nodiscard]] std::size_t vulkan_rounded_rect_perimeter_vertex_count(
    std::size_t corner_segment_count);

void vulkan_append_rounded_rect_vertex(
    std::vector<VulkanRoundedRectVertex>& vertices,
    Point point,
    Color color,
    float coverage);

void vulkan_append_rounded_rect_contour(
    std::vector<VulkanRoundedRectVertex>& vertices,
    Rect rect,
    BorderRadii radii,
    std::size_t corner_segment_count,
    Color color,
    float coverage);

} // namespace cgpui
