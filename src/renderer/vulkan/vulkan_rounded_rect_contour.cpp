#include "vulkan_rounded_rect_contour_internal.hpp"
#include "../planning/rounded_rect_contour_internal.hpp"

namespace cgpui {

std::size_t vulkan_rounded_rect_perimeter_vertex_count(std::size_t segments) {
  return rounded_rect_perimeter_vertex_count(segments);
}
void vulkan_append_rounded_rect_vertex(std::vector<VulkanRoundedRectVertex>& vertices,
    Point point, Color color, float coverage) {
  append_rounded_rect_vertex(vertices, point, color, coverage);
}
void vulkan_append_rounded_rect_contour(std::vector<VulkanRoundedRectVertex>& vertices,
    Rect rect, BorderRadii radii, std::size_t segments, Color color, float coverage) {
  append_rounded_rect_contour(vertices, rect, radii, segments, color, coverage);
}

} // namespace cgpui
