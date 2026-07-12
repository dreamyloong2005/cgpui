#include "cgpui/ui/style_animation.hpp"
#include "style_tween_geometry_internal.hpp"

namespace cgpui {

Style tween(const Style& from, const Style& to, float progress) {
  const float clamped = clamp_animation_progress(progress);
  Style style = clamped < 1.0F ? from : to;
  if (from.background_color.has_value() && to.background_color.has_value()) {
    style.background_color =
        tween(*from.background_color, *to.background_color, clamped);
  }
  if (from.foreground_color.has_value() && to.foreground_color.has_value()) {
    style.foreground_color =
        tween(*from.foreground_color, *to.foreground_color, clamped);
  }
  if (from.border_color.has_value() && to.border_color.has_value()) {
    style.border_color = tween(*from.border_color, *to.border_color, clamped);
  }
  if (from.box_shadow.has_value() && to.box_shadow.has_value()) {
    style.box_shadow =
        detail::tween_style_shadow(*from.box_shadow, *to.box_shadow, clamped);
  }
  if (from.clip_rect.has_value() && to.clip_rect.has_value()) {
    style.clip_rect =
        detail::tween_style_rect(*from.clip_rect, *to.clip_rect, clamped);
  }
  style.preferred_size =
      detail::tween_style_size(from.preferred_size, to.preferred_size, clamped);
  style.min_size = detail::tween_style_size(from.min_size, to.min_size, clamped);
  style.max_size = detail::tween_style_size(from.max_size, to.max_size, clamped);
  style.percentage_size = detail::tween_style_percentage_size(
      from.percentage_size, to.percentage_size, clamped);
  style.padding = detail::tween_style_edges(from.padding, to.padding, clamped);
  style.margin = detail::tween_style_edges(from.margin, to.margin, clamped);
  style.border_width =
      detail::tween_style_edges(from.border_width, to.border_width, clamped);
  style.border_radius =
      detail::tween_style_radii(from.border_radius, to.border_radius, clamped);
  style.gap = detail::tween_style_float(from.gap, to.gap, clamped);
  style.flex_grow =
      detail::tween_style_float(from.flex_grow, to.flex_grow, clamped);
  style.flex_shrink =
      detail::tween_style_float(from.flex_shrink, to.flex_shrink, clamped);
  style.inset = detail::tween_style_edges(from.inset, to.inset, clamped);
  style.font_size =
      detail::tween_style_float(from.font_size, to.font_size, clamped);
  style.opacity = tween_opacity(from.opacity, to.opacity, clamped);
  style.transform = tween(from.transform, to.transform, clamped);
  return style;
}

Style StyleTween::value_at(float progress) const {
  return tween(from, to, curve.value_at(progress, easing));
}

} // namespace cgpui
