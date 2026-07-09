#include "vulkan_rounded_rect_contour_internal.hpp"

#include "vulkan_rounded_rect_antialiasing_internal.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace cgpui {
namespace {

std::array<float, 4> vertex_color(Color color) {
  return {color.r, color.g, color.b, color.a};
}

void append_corner_arc(
    std::vector<VulkanRoundedRectVertex>& vertices,
    Point center,
    float radius,
    float start_angle,
    float end_angle,
    std::size_t segments,
    Color color,
    float coverage) {
  for (std::size_t index = 0; index <= segments; ++index) {
    const float amount =
        static_cast<float>(index) / static_cast<float>(segments);
    const float angle = start_angle + (end_angle - start_angle) * amount;
    vulkan_append_rounded_rect_vertex(
        vertices,
        Point{
            .x = center.x + std::cos(angle) * radius,
            .y = center.y + std::sin(angle) * radius,
        },
        color,
        coverage);
  }
}

} // namespace

std::size_t vulkan_rounded_rect_perimeter_vertex_count(
    std::size_t corner_segment_count) {
  return 4 * (std::max<std::size_t>(1, corner_segment_count) + 1);
}

void vulkan_append_rounded_rect_vertex(
    std::vector<VulkanRoundedRectVertex>& vertices,
    Point point,
    Color color,
    float coverage) {
  vertices.push_back(VulkanRoundedRectVertex{
      .position = {point.x, point.y},
      .color = vertex_color(color),
      .coverage = vulkan_rounded_rect_coverage(coverage),
  });
}

void vulkan_append_rounded_rect_contour(
    std::vector<VulkanRoundedRectVertex>& vertices,
    Rect rect,
    BorderRadii radii,
    std::size_t corner_segment_count,
    Color color,
    float coverage) {
  const std::size_t segments =
      std::max<std::size_t>(1, corner_segment_count);
  const float left = rect.origin.x;
  const float top = rect.origin.y;
  const float right = left + rect.size.width;
  const float bottom = top + rect.size.height;
  constexpr float pi = std::numbers::pi_v<float>;
  append_corner_arc(
      vertices,
      Point{.x = left + radii.top_left, .y = top + radii.top_left},
      radii.top_left, pi, 1.5F * pi, segments, color, coverage);
  append_corner_arc(
      vertices,
      Point{.x = right - radii.top_right, .y = top + radii.top_right},
      radii.top_right, 1.5F * pi, 2.0F * pi, segments, color, coverage);
  append_corner_arc(
      vertices,
      Point{.x = right - radii.bottom_right, .y = bottom - radii.bottom_right},
      radii.bottom_right, 0.0F, 0.5F * pi, segments, color, coverage);
  append_corner_arc(
      vertices,
      Point{.x = left + radii.bottom_left, .y = bottom - radii.bottom_left},
      radii.bottom_left, 0.5F * pi, pi, segments, color, coverage);
}

} // namespace cgpui
