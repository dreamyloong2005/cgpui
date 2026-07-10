#include "vulkan_solid_rect_geometry_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"
#include "vulkan_composition_opacity_internal.hpp"

namespace cgpui {
namespace {

[[nodiscard]] VulkanRoundedRectVertex solid_vertex(
    float x,
    float y,
    Color color) {
  return VulkanRoundedRectVertex{
      .position = {x, y},
      .color = {color.r, color.g, color.b, color.a},
      .coverage = 1.0F,
  };
}

} // namespace

VulkanRoundedRectGeometry vulkan_build_solid_rect_geometry(
    std::span<const SolidRect> rects) {
  VulkanRoundedRectGeometry geometry;
  std::size_t draw_count = 0;
  for (const SolidRect& rect : rects) {
    draw_count += rect.rect.size.width > 0.0F && rect.rect.size.height > 0.0F;
  }
  geometry.vertices.reserve(draw_count * 4);
  geometry.indices.reserve(draw_count * 6);
  geometry.draws.reserve(draw_count);

  for (std::size_t source_index = 0; source_index < rects.size(); ++source_index) {
    const SolidRect& rect = rects[source_index];
    if (rect.rect.size.width <= 0.0F || rect.rect.size.height <= 0.0F) {
      continue;
    }
    const Color color = vulkan_apply_composed_opacity(
        rect.color, rect.metadata, rect.composition_stack);
    const float left = rect.rect.origin.x;
    const float top = rect.rect.origin.y;
    const float right = left + rect.rect.size.width;
    const float bottom = top + rect.rect.size.height;
    const std::size_t first_vertex = geometry.vertices.size();
    const std::size_t first_index = geometry.indices.size();
    geometry.vertices.insert(
        geometry.vertices.end(),
        {solid_vertex(left, top, color),
         solid_vertex(right, top, color),
         solid_vertex(right, bottom, color),
         solid_vertex(left, bottom, color)});
    const auto base = static_cast<std::uint32_t>(first_vertex);
    geometry.indices.insert(
        geometry.indices.end(),
        {base, base + 1, base + 2, base, base + 2, base + 3});
    geometry.draws.push_back(VulkanRoundedRectDrawRange{
        .source_index = source_index,
        .first_vertex = first_vertex,
        .vertex_count = 4,
        .first_index = first_index,
        .index_count = 6,
        .clip_rect = vulkan_resolve_effective_clip_rect(
            rect.clip_rect, rect.clip_stack),
    });
  }
  return geometry;
}

} // namespace cgpui
