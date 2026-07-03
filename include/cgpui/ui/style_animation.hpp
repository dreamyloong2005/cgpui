#pragma once

#include "cgpui/ui/style_box.hpp"

namespace cgpui {

[[nodiscard]] Style tween(const Style& from, const Style& to, float progress);

struct StyleTween {
  Style from;
  Style to;
  AnimationEasing easing = AnimationEasing::linear;

  [[nodiscard]] Style value_at(float progress) const;
};

} // namespace cgpui
