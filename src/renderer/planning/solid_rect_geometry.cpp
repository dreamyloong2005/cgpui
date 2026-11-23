#include "cgpui/renderer/planning/solid_rect_geometry.hpp"

#include "cgpui/renderer/planning/clip_scissor.hpp"
#include "cgpui/renderer/planning/composition_opacity.hpp"
#include "cgpui/renderer/planning/composition_transform.hpp"

namespace cgpui {
namespace {

[[nodiscard]] RectVertex solid_vertex(
    Point point,
    Color color) {
  return RectVertex{
      .position = {point.x, point.y},
      .color = {color.r, color.g, color.b, color.a},
      .coverage = 1.0F,
  };
}

} // namespace

RectGeometry build_solid_rect_geometry(
    std::span<const SolidRect> rects) {
  RectGeometry geometry;
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
    const Color color = apply_composed_opacity(
        rect.color, rect.metadata, rect.composition_stack);
    const float left = rect.rect.origin.x;
    const float top = rect.rect.origin.y;
    const float right = left + rect.rect.size.width;
    const float bottom = top + rect.rect.size.height;
    const Point top_left = apply_composed_transform(
        Point{.x = left, .y = top}, rect.metadata, rect.composition_stack);
    const Point top_right = apply_composed_transform(
        Point{.x = right, .y = top}, rect.metadata, rect.composition_stack);
    const Point bottom_right = apply_composed_transform(
        Point{.x = right, .y = bottom}, rect.metadata, rect.composition_stack);
    const Point bottom_left = apply_composed_transform(
        Point{.x = left, .y = bottom}, rect.metadata, rect.composition_stack);
    const std::size_t first_vertex = geometry.vertices.size();
    const std::size_t first_index = geometry.indices.size();
    geometry.vertices.insert(
        geometry.vertices.end(),
        {solid_vertex(top_left, color),
         solid_vertex(top_right, color),
         solid_vertex(bottom_right, color),
         solid_vertex(bottom_left, color)});
    const auto base = static_cast<std::uint32_t>(first_vertex);
    geometry.indices.insert(
        geometry.indices.end(),
        {base, base + 1, base + 2, base, base + 2, base + 3});
    geometry.draws.push_back(RectDrawRange{
        .source_index = source_index,
        .first_vertex = first_vertex,
        .vertex_count = 4,
        .first_index = first_index,
        .index_count = 6,
        .clip_rect = resolve_effective_clip_rect(
            rect.clip_rect, rect.clip_stack),
    });
  }
  return geometry;
}

} // namespace cgpui
