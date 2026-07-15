#include "metal_renderer_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {
namespace {

AffineTransform resolve_transform(
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack) {
  return composition_stack.empty()
             ? metadata.transform
             : composition_stack.current_metadata.transform;
}

float resolve_opacity(
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack) {
  const float value = composition_stack.empty()
                          ? metadata.opacity
                          : composition_stack.current_metadata.opacity;
  return std::isfinite(value) ? std::clamp(value, 0.0F, 1.0F) : 1.0F;
}

Point transform_point(Point point, AffineTransform transform) {
  return Point{
      .x = transform.scale_x * point.x + transform.skew_x * point.y +
           transform.translate_x,
      .y = transform.skew_y * point.x + transform.scale_y * point.y +
           transform.translate_y};
}

Rect intersect_rects(Rect first, Rect second) {
  const float left = std::max(first.origin.x, second.origin.x);
  const float top = std::max(first.origin.y, second.origin.y);
  const float right = std::min(
      first.origin.x + first.size.width,
      second.origin.x + second.size.width);
  const float bottom = std::min(
      first.origin.y + first.size.height,
      second.origin.y + second.size.height);
  return Rect{
      .origin = {left, top},
      .size = {std::max(0.0F, right - left),
               std::max(0.0F, bottom - top)}};
}

}  // namespace

std::array<MetalVertex, 4> metal_rect_vertices(
    Rect rect,
    Color color,
    PaintMetadata metadata,
    const RendererCompositionStackRecord& composition_stack,
    Rect texture_coordinates) {
  const AffineTransform transform = resolve_transform(metadata, composition_stack);
  color.a *= resolve_opacity(metadata, composition_stack);
  const float left = rect.origin.x;
  const float top = rect.origin.y;
  const float right = left + rect.size.width;
  const float bottom = top + rect.size.height;
  const float u0 = texture_coordinates.origin.x;
  const float v0 = texture_coordinates.origin.y;
  const float u1 = u0 + texture_coordinates.size.width;
  const float v1 = v0 + texture_coordinates.size.height;
  const simd_float4 encoded_color{color.r, color.g, color.b, color.a};
  const Point top_left = transform_point({left, top}, transform);
  const Point top_right = transform_point({right, top}, transform);
  const Point bottom_left = transform_point({left, bottom}, transform);
  const Point bottom_right = transform_point({right, bottom}, transform);
  return {{
      {.position = {top_left.x, top_left.y},
       .texture_coordinate = {u0, v0},
       .color = encoded_color},
      {.position = {top_right.x, top_right.y},
       .texture_coordinate = {u1, v0},
       .color = encoded_color},
      {.position = {bottom_left.x, bottom_left.y},
       .texture_coordinate = {u0, v1},
       .color = encoded_color},
      {.position = {bottom_right.x, bottom_right.y},
       .texture_coordinate = {u1, v1},
       .color = encoded_color},
  }};
}

MetalClipResolution metal_resolve_clip(
    Size framebuffer_size,
    std::optional<Rect> clip_rect,
    const RendererClipStackRecord& clip_stack) {
  std::optional<Rect> effective;
  const auto append = [&](Rect clip) {
    effective = effective ? intersect_rects(*effective, clip) : clip;
  };
  for (Rect clip : clip_stack.clips) append(clip);
  if (clip_stack.current_clip_rect) append(*clip_stack.current_clip_rect);
  if (clip_rect) append(*clip_rect);

  Rect resolved{
      .origin = {},
      .size = framebuffer_size};
  if (effective) resolved = intersect_rects(resolved, *effective);
  const float left = std::clamp(resolved.origin.x, 0.0F, framebuffer_size.width);
  const float top = std::clamp(resolved.origin.y, 0.0F, framebuffer_size.height);
  const float right = std::clamp(
      resolved.origin.x + resolved.size.width, 0.0F, framebuffer_size.width);
  const float bottom = std::clamp(
      resolved.origin.y + resolved.size.height, 0.0F, framebuffer_size.height);
  if (!(right > left && bottom > top)) return {};
  const std::size_t x = static_cast<std::size_t>(std::floor(left));
  const std::size_t y = static_cast<std::size_t>(std::floor(top));
  const std::size_t max_x = static_cast<std::size_t>(std::ceil(right));
  const std::size_t max_y = static_cast<std::size_t>(std::ceil(bottom));
  return MetalClipResolution{
      .scissor = {x, y, max_x - x, max_y - y},
      .visible = true};
}

}  // namespace cgpui
