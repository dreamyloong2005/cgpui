#include "vulkan_rounded_rect_geometry_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"
#include "vulkan_rounded_rect_contour_internal.hpp"
#include "vulkan_rounded_rect_indices_internal.hpp"
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
    if (!draw.fill_enabled && !stroked) {
      continue;
    }
    if (draw.fill_enabled) {
      total_vertex_count += perimeter_count + 1;
      total_index_count += perimeter_count * 3;
    }
    if (stroked) {
      total_vertex_count += perimeter_count * 3;
      total_index_count += perimeter_count * 12;
    } else {
      total_vertex_count += perimeter_count;
      total_index_count += perimeter_count * 6;
    }
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
    if (!draw.fill_enabled && !stroke.enabled) {
      continue;
    }

    const std::size_t first_vertex = geometry.vertices.size();
    const std::size_t first_index = geometry.indices.size();
    std::uint32_t center_index = 0;
    std::uint32_t first_fill = 0;
    std::uint32_t first_stroke_inner = 0;
    std::uint32_t first_stroke_outer = 0;
    std::uint32_t first_fringe = 0;

    if (draw.fill_enabled) {
      center_index = static_cast<std::uint32_t>(geometry.vertices.size());
      vulkan_append_rounded_rect_vertex(
          geometry.vertices,
          Point{
              .x = draw.rect.origin.x + draw.rect.size.width * 0.5F,
              .y = draw.rect.origin.y + draw.rect.size.height * 0.5F,
          },
          draw.color,
          1.0F);
      first_fill = static_cast<std::uint32_t>(geometry.vertices.size());
      vulkan_append_rounded_rect_contour(
          geometry.vertices,
          stroke.enabled ? stroke.inner_rect : draw.rect,
          stroke.enabled ? stroke.inner_radii : radii,
          segments,
          draw.color,
          1.0F);
    }

    if (stroke.enabled) {
      first_stroke_inner =
          static_cast<std::uint32_t>(geometry.vertices.size());
      vulkan_append_rounded_rect_contour(
          geometry.vertices,
          stroke.inner_rect,
          stroke.inner_radii,
          segments,
          stroke.color,
          1.0F);
      first_stroke_outer =
          static_cast<std::uint32_t>(geometry.vertices.size());
      vulkan_append_rounded_rect_contour(
          geometry.vertices, draw.rect, radii, segments, stroke.color, 1.0F);
      first_fringe = static_cast<std::uint32_t>(geometry.vertices.size());
      vulkan_append_rounded_rect_contour(
          geometry.vertices,
          expanded_rect(draw.rect, fringe_width),
          expanded_radii(radii, fringe_width),
          segments,
          stroke.color,
          0.0F);
    } else {
      first_fringe = static_cast<std::uint32_t>(geometry.vertices.size());
      vulkan_append_rounded_rect_contour(
          geometry.vertices,
          expanded_rect(draw.rect, fringe_width),
          expanded_radii(radii, fringe_width),
          segments,
          draw.color,
          0.0F);
    }

    if (draw.fill_enabled) {
      vulkan_append_rounded_rect_fan_indices(
          geometry.indices, center_index, first_fill, perimeter_count);
    }
    if (stroke.enabled) {
      vulkan_append_rounded_rect_ring_indices(
          geometry.indices,
          first_stroke_inner,
          first_stroke_outer,
          perimeter_count);
      vulkan_append_rounded_rect_ring_indices(
          geometry.indices, first_stroke_outer, first_fringe, perimeter_count);
    } else {
      vulkan_append_rounded_rect_ring_indices(
          geometry.indices, first_fill, first_fringe, perimeter_count);
    }
    geometry.draws.push_back(VulkanRoundedRectDrawRange{
        .source_index = source_index,
        .first_vertex = first_vertex,
        .vertex_count = geometry.vertices.size() - first_vertex,
        .first_index = first_index,
        .index_count = geometry.indices.size() - first_index,
        .clip_rect = vulkan_resolve_effective_clip_rect(
            draw.clip_rect, draw.clip_stack),
    });
  }
  return geometry;
}

} // namespace cgpui
