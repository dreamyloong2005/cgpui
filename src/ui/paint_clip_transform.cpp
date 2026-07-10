#include "paint_clip_transform.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace cgpui {
namespace {

[[nodiscard]] bool finite_transform(AffineTransform transform) {
  return std::isfinite(transform.scale_x) &&
         std::isfinite(transform.skew_y) &&
         std::isfinite(transform.skew_x) &&
         std::isfinite(transform.scale_y) &&
         std::isfinite(transform.translate_x) &&
         std::isfinite(transform.translate_y);
}

[[nodiscard]] Point transform_point(Point point, AffineTransform transform) {
  return Point{
      .x = transform.scale_x * point.x + transform.skew_x * point.y +
           transform.translate_x,
      .y = transform.skew_y * point.x + transform.scale_y * point.y +
           transform.translate_y,
  };
}

} // namespace

Rect transform_clip_rect_to_framebuffer_aabb(
    Rect clip,
    AffineTransform transform) {
  if (!finite_transform(transform)) {
    return clip;
  }
  const float left = clip.origin.x;
  const float top = clip.origin.y;
  const float right = left + clip.size.width;
  const float bottom = top + clip.size.height;
  const std::array<Point, 4> corners{
      transform_point(Point{.x = left, .y = top}, transform),
      transform_point(Point{.x = right, .y = top}, transform),
      transform_point(Point{.x = right, .y = bottom}, transform),
      transform_point(Point{.x = left, .y = bottom}, transform),
  };
  float min_x = corners[0].x;
  float min_y = corners[0].y;
  float max_x = corners[0].x;
  float max_y = corners[0].y;
  for (const Point point : corners) {
    if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
      return clip;
    }
    min_x = std::min(min_x, point.x);
    min_y = std::min(min_y, point.y);
    max_x = std::max(max_x, point.x);
    max_y = std::max(max_y, point.y);
  }
  return Rect{
      .origin = {.x = min_x, .y = min_y},
      .size = {.width = max_x - min_x, .height = max_y - min_y},
  };
}

} // namespace cgpui
