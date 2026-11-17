#include "vulkan_box_shadow_geometry_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"
#include "vulkan_composition_opacity_internal.hpp"
#include "vulkan_composition_transform_internal.hpp"
#include "vulkan_rounded_rect_radii_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {
namespace {

std::array<float, 4> rect_bounds(Rect rect) {
  return {
      rect.origin.x,
      rect.origin.y,
      rect.origin.x + rect.size.width,
      rect.origin.y + rect.size.height,
  };
}

std::array<float, 4> radii_values(BorderRadii radii) {
  return {radii.top_left, radii.top_right, radii.bottom_right,
          radii.bottom_left};
}

BorderRadii spread_radii(BorderRadii radii, float spread) {
  return BorderRadii::corners(
      vulkan_box_shadow_spread_radius(radii.top_left, spread),
      vulkan_box_shadow_spread_radius(radii.top_right, spread),
      vulkan_box_shadow_spread_radius(radii.bottom_right, spread),
      vulkan_box_shadow_spread_radius(radii.bottom_left, spread));
}

} // namespace

float vulkan_box_shadow_sigma(float blur_radius) {
  return std::isfinite(blur_radius) ? std::max(0.0F, blur_radius) * 0.5F
                                    : 0.0F;
}

float vulkan_box_shadow_spread_radius(float radius, float spread) {
  if (radius <= 0.0F) {
    return 0.0F;
  }
  if (spread <= 0.0F || radius >= spread) {
    return std::max(0.0F, radius + spread);
  }
  const float ratio = radius / spread - 1.0F;
  return radius + spread * (1.0F + ratio * ratio * ratio);
}

VulkanBoxShadowGeometry vulkan_build_box_shadow_geometry(
    std::span<const BoxShadowDraw> shadows) {
  VulkanBoxShadowGeometry geometry;
  geometry.vertices.reserve(shadows.size() * 4);
  geometry.indices.reserve(shadows.size() * 6);
  geometry.draws.reserve(shadows.size());
  for (std::size_t source_index = 0; source_index < shadows.size();
       ++source_index) {
    const BoxShadowDraw& draw = shadows[source_index];
    const Color color = vulkan_apply_composed_opacity(
        draw.shadow.color, draw.metadata, draw.composition_stack);
    const float spread = std::isfinite(draw.shadow.spread_radius)
                             ? draw.shadow.spread_radius
                             : 0.0F;
    const Rect shape{
        .origin = {.x = draw.rect.origin.x + draw.shadow.offset.x - spread,
                   .y = draw.rect.origin.y + draw.shadow.offset.y - spread},
        .size = {.width = draw.rect.size.width + spread * 2.0F,
                 .height = draw.rect.size.height + spread * 2.0F},
    };
    if (draw.rect.size.width <= 0.0F || draw.rect.size.height <= 0.0F ||
        shape.size.width <= 0.0F || shape.size.height <= 0.0F ||
        color.a <= 0.0F) {
      continue;
    }
    const BorderRadii box_radii =
        vulkan_resolve_rounded_rect_radii(draw.rect.size, draw.radius).radii;
    const BorderRadii shape_radii = vulkan_resolve_rounded_rect_radii(
        shape.size, spread_radii(box_radii, spread)).radii;
    const float sigma = vulkan_box_shadow_sigma(draw.shadow.blur_radius);
    const float margin = sigma * 3.0F + 1.0F;
    const std::array<float, 4> quad{
        shape.origin.x - margin,
        shape.origin.y - margin,
        shape.origin.x + shape.size.width + margin,
        shape.origin.y + shape.size.height + margin,
    };
    const auto first_vertex =
        static_cast<std::uint32_t>(geometry.vertices.size());
    const std::size_t first_index = geometry.indices.size();
    const std::array<Point, 4> corners{
        Point{.x = quad[0], .y = quad[1]},
        Point{.x = quad[2], .y = quad[1]},
        Point{.x = quad[2], .y = quad[3]},
        Point{.x = quad[0], .y = quad[3]},
    };
    for (const Point corner : corners) {
      const Point position = vulkan_apply_composed_transform(
          corner, draw.metadata, draw.composition_stack);
      geometry.vertices.push_back(VulkanBoxShadowVertex{
          .position = {position.x, position.y},
          .local = {corner.x, corner.y},
          .color = {color.r, color.g, color.b, color.a},
          .shadow_rect = rect_bounds(shape),
          .shadow_radii = radii_values(shape_radii),
          .box_rect = rect_bounds(draw.rect),
          .box_radii = radii_values(box_radii),
          .sigma = sigma,
      });
    }
    for (const std::uint32_t offset : {0U, 1U, 2U, 2U, 3U, 0U}) {
      geometry.indices.push_back(first_vertex + offset);
    }
    geometry.draws.push_back(VulkanRoundedRectDrawRange{
        .source_index = source_index,
        .first_vertex = first_vertex,
        .vertex_count = 4,
        .first_index = first_index,
        .index_count = 6,
        .clip_rect = vulkan_resolve_effective_clip_rect(
            draw.clip_rect, draw.clip_stack),
    });
  }
  return geometry;
}

} // namespace cgpui
