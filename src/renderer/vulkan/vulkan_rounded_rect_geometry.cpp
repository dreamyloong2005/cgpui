#include "vulkan_rounded_rect_geometry_internal.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace cgpui {
namespace {

std::array<float, 4> vertex_color(Color color) {
  return {color.r, color.g, color.b, color.a};
}

float bounded_radius(float radius, Size size) {
  return std::clamp(
      radius,
      0.0F,
      std::min(size.width, size.height) * 0.5F);
}

std::size_t corner_vertex_count(float radius, std::size_t segments) {
  return radius > 0.0F ? segments + 1 : 1;
}

BorderRadii bounded_radii(const RoundedRectDraw& draw) {
  return BorderRadii{
      .top_left = bounded_radius(draw.radius.top_left, draw.rect.size),
      .top_right = bounded_radius(draw.radius.top_right, draw.rect.size),
      .bottom_right = bounded_radius(draw.radius.bottom_right, draw.rect.size),
      .bottom_left = bounded_radius(draw.radius.bottom_left, draw.rect.size),
  };
}

std::size_t perimeter_vertex_count(
    BorderRadii radii,
    std::size_t segments) {
  return corner_vertex_count(radii.top_left, segments) +
         corner_vertex_count(radii.top_right, segments) +
         corner_vertex_count(radii.bottom_right, segments) +
         corner_vertex_count(radii.bottom_left, segments);
}

void append_vertex(
    std::vector<VulkanRoundedRectVertex>& vertices,
    Point point,
    Color color) {
  vertices.push_back(VulkanRoundedRectVertex{
      .position = {point.x, point.y},
      .color = vertex_color(color),
  });
}

void append_corner_arc(
    std::vector<VulkanRoundedRectVertex>& vertices,
    Point center,
    float radius,
    float start_angle,
    float end_angle,
    std::size_t segments,
    Color color) {
  if (radius <= 0.0F) {
    append_vertex(vertices, center, color);
    return;
  }
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
        color);
  }
}

} // namespace

VulkanRoundedRectGeometry vulkan_build_rounded_rect_geometry(
    std::span<const RoundedRectDraw> rounded_rects,
    std::size_t corner_segment_count) {
  VulkanRoundedRectGeometry geometry;
  const std::size_t segments = std::max<std::size_t>(1, corner_segment_count);
  std::size_t total_vertex_count = 0;
  std::size_t total_index_count = 0;
  std::size_t total_draw_count = 0;
  for (const RoundedRectDraw& draw : rounded_rects) {
    if (draw.rect.size.width <= 0.0F || draw.rect.size.height <= 0.0F) {
      continue;
    }
    const std::size_t perimeter_count =
        perimeter_vertex_count(bounded_radii(draw), segments);
    total_vertex_count += perimeter_count + 1;
    total_index_count += perimeter_count * 3;
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
    const BorderRadii radii = bounded_radii(draw);
    const std::size_t perimeter_count =
        perimeter_vertex_count(radii, segments);
    const std::size_t first_vertex = geometry.vertices.size();
    const std::size_t first_index = geometry.indices.size();
    const float left = draw.rect.origin.x;
    const float top = draw.rect.origin.y;
    const float right = left + draw.rect.size.width;
    const float bottom = top + draw.rect.size.height;
    append_vertex(
        geometry.vertices,
        Point{.x = (left + right) * 0.5F, .y = (top + bottom) * 0.5F},
        draw.color);
    constexpr float pi = std::numbers::pi_v<float>;
    append_corner_arc(
        geometry.vertices,
        Point{.x = left + radii.top_left, .y = top + radii.top_left},
        radii.top_left, pi, 1.5F * pi, segments, draw.color);
    append_corner_arc(
        geometry.vertices,
        Point{.x = right - radii.top_right, .y = top + radii.top_right},
        radii.top_right, 1.5F * pi, 2.0F * pi, segments, draw.color);
    append_corner_arc(
        geometry.vertices,
        Point{.x = right - radii.bottom_right, .y = bottom - radii.bottom_right},
        radii.bottom_right, 0.0F, 0.5F * pi, segments, draw.color);
    append_corner_arc(
        geometry.vertices,
        Point{.x = left + radii.bottom_left, .y = bottom - radii.bottom_left},
        radii.bottom_left, 0.5F * pi, pi, segments, draw.color);

    const auto center = static_cast<std::uint32_t>(first_vertex);
    const auto first_perimeter = static_cast<std::uint32_t>(first_vertex + 1);
    for (std::size_t index = 0; index < perimeter_count; ++index) {
      geometry.indices.push_back(center);
      geometry.indices.push_back(
          first_perimeter + static_cast<std::uint32_t>(index));
      geometry.indices.push_back(
          first_perimeter +
          static_cast<std::uint32_t>((index + 1) % perimeter_count));
    }
    geometry.draws.push_back(VulkanRoundedRectDrawRange{
        .source_index = source_index,
        .first_vertex = first_vertex,
        .vertex_count = perimeter_count + 1,
        .first_index = first_index,
        .index_count = perimeter_count * 3,
    });
  }
  return geometry;
}

} // namespace cgpui
