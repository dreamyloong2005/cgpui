#include "vulkan_rounded_rect_geometry_internal.hpp"

#include "vulkan_rounded_rect_contour_internal.hpp"
#include "vulkan_rounded_rect_radii_internal.hpp"
#include "vulkan_rounded_rect_stroke_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

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

void append_fan_indices(
    std::vector<std::uint32_t>& indices,
    std::uint32_t center,
    std::uint32_t first_ring,
    std::size_t perimeter_count) {
  for (std::size_t index = 0; index < perimeter_count; ++index) {
    const auto current = static_cast<std::uint32_t>(index);
    const auto next = static_cast<std::uint32_t>((index + 1) % perimeter_count);
    indices.insert(
        indices.end(), {center, first_ring + current, first_ring + next});
  }
}

void append_ring_indices(
    std::vector<std::uint32_t>& indices,
    std::uint32_t first_inner,
    std::uint32_t first_outer,
    std::size_t perimeter_count) {
  for (std::size_t index = 0; index < perimeter_count; ++index) {
    const auto current = static_cast<std::uint32_t>(index);
    const auto next = static_cast<std::uint32_t>((index + 1) % perimeter_count);
    indices.insert(
        indices.end(),
        {first_inner + current,
         first_outer + current,
         first_outer + next,
         first_inner + current,
         first_outer + next,
         first_inner + next});
  }
}

} // namespace

VulkanRoundedRectGeometry vulkan_build_rounded_rect_geometry(
    std::span<const RoundedRectDraw> rounded_rects,
    std::size_t corner_segment_count,
    VulkanRoundedRectAntialiasingPolicy antialiasing) {
  VulkanRoundedRectGeometry geometry;
  const std::size_t segments = std::max<std::size_t>(1, corner_segment_count);
  const std::size_t perimeter_count =
      vulkan_rounded_rect_perimeter_vertex_count(segments);
  const float fringe_width = std::max(0.0F, antialiasing.fringe_width);
  std::size_t total_vertex_count = 0;
  std::size_t total_index_count = 0;
  std::size_t total_draw_count = 0;
  for (const RoundedRectDraw& draw : rounded_rects) {
    if (draw.rect.size.width <= 0.0F || draw.rect.size.height <= 0.0F) {
      continue;
    }
    const bool stroked =
        draw.border_color.has_value() && draw.border_width > 0.0F;
    total_vertex_count += perimeter_count * (stroked ? 4 : 2) + 1;
    total_index_count += perimeter_count * (stroked ? 15 : 9);
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
    const VulkanRoundedRectStrokeResolution stroke =
        vulkan_resolve_rounded_rect_stroke(draw, radii);
    const std::size_t first_vertex = geometry.vertices.size();
    const std::size_t first_index = geometry.indices.size();
    const Point center{
        .x = draw.rect.origin.x + draw.rect.size.width * 0.5F,
        .y = draw.rect.origin.y + draw.rect.size.height * 0.5F,
    };
    vulkan_append_rounded_rect_vertex(
        geometry.vertices, center, draw.color, 1.0F);

    if (stroke.enabled) {
      vulkan_append_rounded_rect_contour(
          geometry.vertices,
          stroke.inner_rect,
          stroke.inner_radii,
          segments,
          draw.color,
          1.0F);
      vulkan_append_rounded_rect_contour(
          geometry.vertices,
          stroke.inner_rect,
          stroke.inner_radii,
          segments,
          stroke.color,
          1.0F);
      vulkan_append_rounded_rect_contour(
          geometry.vertices, draw.rect, radii, segments, stroke.color, 1.0F);
      vulkan_append_rounded_rect_contour(
          geometry.vertices,
          expanded_rect(draw.rect, fringe_width),
          expanded_radii(radii, fringe_width),
          segments,
          stroke.color,
          0.0F);
    } else {
      vulkan_append_rounded_rect_contour(
          geometry.vertices, draw.rect, radii, segments, draw.color, 1.0F);
      vulkan_append_rounded_rect_contour(
          geometry.vertices,
          expanded_rect(draw.rect, fringe_width),
          expanded_radii(radii, fringe_width),
          segments,
          draw.color,
          0.0F);
    }

    const auto center_index = static_cast<std::uint32_t>(first_vertex);
    const auto first_fill = static_cast<std::uint32_t>(first_vertex + 1);
    append_fan_indices(
        geometry.indices, center_index, first_fill, perimeter_count);
    if (stroke.enabled) {
      const auto first_stroke_inner =
          static_cast<std::uint32_t>(first_vertex + 1 + perimeter_count);
      const auto first_stroke_outer = static_cast<std::uint32_t>(
          first_vertex + 1 + perimeter_count * 2);
      const auto first_fringe = static_cast<std::uint32_t>(
          first_vertex + 1 + perimeter_count * 3);
      append_ring_indices(
          geometry.indices,
          first_stroke_inner,
          first_stroke_outer,
          perimeter_count);
      append_ring_indices(
          geometry.indices, first_stroke_outer, first_fringe, perimeter_count);
    } else {
      const auto first_fringe =
          static_cast<std::uint32_t>(first_vertex + 1 + perimeter_count);
      append_ring_indices(
          geometry.indices, first_fill, first_fringe, perimeter_count);
    }
    geometry.draws.push_back(VulkanRoundedRectDrawRange{
        .source_index = source_index,
        .first_vertex = first_vertex,
        .vertex_count = geometry.vertices.size() - first_vertex,
        .first_index = first_index,
        .index_count = geometry.indices.size() - first_index,
    });
  }
  return geometry;
}

} // namespace cgpui
