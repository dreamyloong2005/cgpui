#include "cgpui/ui/style_animation.hpp"

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
  style.opacity = tween_opacity(from.opacity, to.opacity, clamped);
  style.transform = tween(from.transform, to.transform, clamped);
  return style;
}

Style StyleTween::value_at(float progress) const {
  return tween(from, to, curve.value_at(progress, easing));
}

} // namespace cgpui
