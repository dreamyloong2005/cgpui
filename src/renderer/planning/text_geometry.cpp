#include "cgpui/renderer/planning/text_geometry.hpp"

#include "cgpui/renderer/planning/composition_transform.hpp"

namespace cgpui {
namespace {

TextVertex make_text_vertex(
    float x,
    float y,
    float atlas_u,
    float atlas_v,
    const std::array<float, 4>& color) {
  return TextVertex{
      .position = {x, y},
      .atlas_uv = {atlas_u, atlas_v},
      .color = color,
  };
}

void append_text_quad_vertices(
    std::vector<TextVertex>& vertices,
    const TexturedGlyphQuad& quad,
    TextPositioningPolicy positioning_policy) {
  const Rect device_bounds =
      position_text_bounds(quad.device_bounds, positioning_policy);
  const float left = device_bounds.origin.x;
  const float top = device_bounds.origin.y;
  const float right = left + device_bounds.size.width;
  const float bottom = top + device_bounds.size.height;
  const float atlas_left = quad.atlas_uv_bounds.origin.x;
  const float atlas_top = quad.atlas_uv_bounds.origin.y;
  const float atlas_right = atlas_left + quad.atlas_uv_bounds.size.width;
  const float atlas_bottom = atlas_top + quad.atlas_uv_bounds.size.height;
  const std::array<float, 4> color{
      quad.color.r,
      quad.color.g,
      quad.color.b,
      quad.color.a,
  };

  const Point top_left_point = apply_composed_transform(
      Point{.x = left, .y = top}, quad.metadata, quad.composition_stack);
  const Point top_right_point = apply_composed_transform(
      Point{.x = right, .y = top}, quad.metadata, quad.composition_stack);
  const Point bottom_right_point = apply_composed_transform(
      Point{.x = right, .y = bottom}, quad.metadata, quad.composition_stack);
  const Point bottom_left_point = apply_composed_transform(
      Point{.x = left, .y = bottom}, quad.metadata, quad.composition_stack);
  const TextVertex top_left = make_text_vertex(
      top_left_point.x, top_left_point.y, atlas_left, atlas_top, color);
  const TextVertex top_right = make_text_vertex(
      top_right_point.x, top_right_point.y, atlas_right, atlas_top, color);
  const TextVertex bottom_right = make_text_vertex(
      bottom_right_point.x,
      bottom_right_point.y,
      atlas_right,
      atlas_bottom,
      color);
  const TextVertex bottom_left = make_text_vertex(
      bottom_left_point.x,
      bottom_left_point.y,
      atlas_left,
      atlas_bottom,
      color);
  vertices.insert(
      vertices.end(),
      {top_left, top_right, bottom_right, top_left, bottom_right, bottom_left});
}

} // namespace

std::vector<TextVertex> build_text_vertices(
    std::span<const TexturedGlyphQuad> quads,
    TextPositioningPolicy positioning_policy) {
  std::vector<TextVertex> vertices;
  vertices.reserve(quads.size() * text_vertices_per_quad);
  for (const TexturedGlyphQuad& quad : quads) {
    append_text_quad_vertices(vertices, quad, positioning_policy);
  }
  return vertices;
}

} // namespace cgpui
