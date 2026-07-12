#pragma once

#include "cgpui/ui/animation_curve.hpp"
#include "cgpui/ui/style_box.hpp"

namespace cgpui {

[[nodiscard]] Style tween(const Style& from, const Style& to, float progress);

struct StyleTween {
  Style from;
  Style to;
  AnimationEasing easing = AnimationEasing::linear;
  AnimationCurve curve;

  [[nodiscard]] Style value_at(float progress) const;
};

} // namespace cgpui
