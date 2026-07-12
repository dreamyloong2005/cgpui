#include "style_tween_geometry_internal.hpp"

#include <cmath>

namespace cgpui::detail {

float tween_style_float(float from, float to, float progress) {
  const float clamped = clamp_animation_progress(progress);
  if (clamped <= 0.0F || from == to) return from;
  if (clamped >= 1.0F) return to;
  if (!std::isfinite(from) || !std::isfinite(to)) return from;
  return tween(from, to, clamped);
}

Size tween_style_size(Size from, Size to, float progress) {
  return Size{
      .width = tween_style_float(from.width, to.width, progress),
      .height = tween_style_float(from.height, to.height, progress),
  };
}

Point tween_style_point(Point from, Point to, float progress) {
  return Point{
      .x = tween_style_float(from.x, to.x, progress),
      .y = tween_style_float(from.y, to.y, progress),
  };
}

Rect tween_style_rect(Rect from, Rect to, float progress) {
  return Rect{
      .origin = tween_style_point(from.origin, to.origin, progress),
      .size = tween_style_size(from.size, to.size, progress),
  };
}

EdgeSizes tween_style_edges(
    EdgeSizes from,
    EdgeSizes to,
    float progress) {
  return EdgeSizes{
      .top = tween_style_float(from.top, to.top, progress),
      .right = tween_style_float(from.right, to.right, progress),
      .bottom = tween_style_float(from.bottom, to.bottom, progress),
      .left = tween_style_float(from.left, to.left, progress),
  };
}

BorderRadii tween_style_radii(
    BorderRadii from,
    BorderRadii to,
    float progress) {
  return BorderRadii{
      .top_left = tween_style_float(from.top_left, to.top_left, progress),
      .top_right = tween_style_float(from.top_right, to.top_right, progress),
      .bottom_right =
          tween_style_float(from.bottom_right, to.bottom_right, progress),
      .bottom_left =
          tween_style_float(from.bottom_left, to.bottom_left, progress),
  };
}

BoxShadow tween_style_shadow(
    BoxShadow from,
    BoxShadow to,
    float progress) {
  return BoxShadow{
      .color = tween(from.color, to.color, progress),
      .offset = tween_style_point(from.offset, to.offset, progress),
      .blur_radius =
          tween_style_float(from.blur_radius, to.blur_radius, progress),
      .spread_radius =
          tween_style_float(from.spread_radius, to.spread_radius, progress),
  };
}

PercentageSize tween_style_percentage_size(
    const PercentageSize& from,
    const PercentageSize& to,
    float progress) {
  PercentageSize result = progress < 1.0F ? from : to;
  if (from.width.has_value() && to.width.has_value()) {
    result.width = tween_style_float(*from.width, *to.width, progress);
  }
  if (from.height.has_value() && to.height.has_value()) {
    result.height = tween_style_float(*from.height, *to.height, progress);
  }
  return result;
}

} // namespace cgpui::detail
