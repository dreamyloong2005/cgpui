#pragma once

#include "cgpui/ui/element_animation.hpp"

#include <vector>

namespace cgpui {

struct ElementAnimationStage {
  AnimationOptions animation;
  bool repeating = false;

  [[nodiscard]] ElementAnimationStage repeat() const;
};

[[nodiscard]] AnyElement with_animations(
    ElementKey key,
    std::vector<ElementAnimationStage> stages,
    AnyElement child,
    ElementAnimationCallback callback);

} // namespace cgpui
