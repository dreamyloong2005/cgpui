#include "vulkan_rounded_rect_geometry_internal.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace cgpui {
namespace {

std::array<float, 4> vertex_color(Color color) {
  return {color.r, color.g, color.b, color.a};
}

std::size_t perimeter_vertex_count(std::size_t segments) {
  return 4 * (segments + 1);
}

void append_vertex(
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
    append_vertex(
        vertices,
        Point{
            .x = center.x + std::cos(angle) * radius,
            .y = center.y + std::sin(angle) * radius,
        },
        color,
        coverage);
  }
}

void append_perimeter(
    std::vector<VulkanRoundedRectVertex>& vertices,
    Rect rect,
    BorderRadii radii,
    std::size_t segments,
    Color color,
    float coverage) {
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

Rect expanded_rect(Rect rect, float amount) {
  return Rect{
      .origin = {.x = rect.origin.x - amount, .y = rect.origin.y - amount},
      .size = {.width = rect.size.width + amount * 2.0F,
               .height = rect.size.height + amount * 2.0F},
  };
}

BorderRadii expanded_radii(BorderRadii radii, float amount) {
  return BorderRadii::corners(
      radii.top_left + amount,
      radii.top_right + amount,
      radii.bottom_right + amount,
      radii.bottom_left + amount);
}

} // namespace

VulkanRoundedRectGeometry vulkan_build_rounded_rect_geometry(
    std::span<const RoundedRectDraw> rounded_rects,
    std::size_t corner_segment_count,
    VulkanRoundedRectAntialiasingPolicy antialiasing) {
  VulkanRoundedRectGeometry geometry;
  const std::size_t segments = std::max<std::size_t>(1, corner_segment_count);
  const float fringe_width = std::max(0.0F, antialiasing.fringe_width);
  std::size_t total_vertex_count = 0;
  std::size_t total_index_count = 0;
  std::size_t total_draw_count = 0;
  for (const RoundedRectDraw& draw : rounded_rects) {
    if (draw.rect.size.width <= 0.0F || draw.rect.size.height <= 0.0F) {
      continue;
    }
    const std::size_t perimeter_count = perimeter_vertex_count(segments);
    total_vertex_count += perimeter_count * 2 + 1;
    total_index_count += perimeter_count * 9;
    ++total_draw_count;
  }
  geometry.vertices.reserve(total_vertex_count);
  geometry.indices.reserve(total_index_count);
  geometry.draws.reserve(total_draw_count);

  for (std::size_t source_index = 0; source_index < rounded_rects.size();
       ++source_index) {
    const RoundedRectDraw& draw = rounded_rects[source_index];
    if (draw.rect.size.width <= 0.0F || draw.rect.size.height <= 0.0F) {
      continue;
    }
    const BorderRadii radii =
        vulkan_resolve_rounded_rect_radii(draw.rect.size, draw.radius).radii;
    const std::size_t perimeter_count = perimeter_vertex_count(segments);
    const std::size_t first_vertex = geometry.vertices.size();
    const std::size_t first_index = geometry.indices.size();
    const float left = draw.rect.origin.x;
    const float top = draw.rect.origin.y;
    const float right = left + draw.rect.size.width;
    const float bottom = top + draw.rect.size.height;
    append_vertex(
        geometry.vertices,
        Point{.x = (left + right) * 0.5F, .y = (top + bottom) * 0.5F},
        draw.color,
        1.0F);
    append_perimeter(
        geometry.vertices, draw.rect, radii, segments, draw.color, 1.0F);
    append_perimeter(
        geometry.vertices,
        expanded_rect(draw.rect, fringe_width),
        expanded_radii(radii, fringe_width),
        segments,
        draw.color,
        0.0F);

    const auto center = static_cast<std::uint32_t>(first_vertex);
    const auto first_inner = static_cast<std::uint32_t>(first_vertex + 1);
    const auto first_outer =
        static_cast<std::uint32_t>(first_vertex + 1 + perimeter_count);
    for (std::size_t index = 0; index < perimeter_count; ++index) {
      const auto current = static_cast<std::uint32_t>(index);
      const auto next =
          static_cast<std::uint32_t>((index + 1) % perimeter_count);
      geometry.indices.push_back(center);
      geometry.indices.push_back(first_inner + current);
      geometry.indices.push_back(first_inner + next);
      geometry.indices.push_back(first_inner + current);
      geometry.indices.push_back(first_outer + current);
      geometry.indices.push_back(first_outer + next);
      geometry.indices.push_back(first_inner + current);
      geometry.indices.push_back(first_outer + next);
      geometry.indices.push_back(first_inner + next);
    }
    geometry.draws.push_back(VulkanRoundedRectDrawRange{
        .source_index = source_index,
        .first_vertex = first_vertex,
        .vertex_count = perimeter_count * 2 + 1,
        .first_index = first_index,
        .index_count = perimeter_count * 9,
    });
  }
  return geometry;
}

} // namespace cgpui
