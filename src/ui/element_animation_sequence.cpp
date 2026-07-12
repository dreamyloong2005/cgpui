#include "cgpui/ui/element_animation_sequence.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {

ElementAnimationStage ElementAnimationStage::repeat() const {
  ElementAnimationStage repeated = *this;
  repeated.repeating = true;
  return repeated;
}

AnyElement with_animations(
    ElementKey key,
    std::vector<ElementAnimationStage> stages,
    AnyElement child,
    ElementAnimationCallback callback) {
  const bool invalid_repeating_stage = std::ranges::any_of(
      stages,
      [](const ElementAnimationStage& stage) {
        return stage.repeating && stage.animation.duration_ms == 0;
      });
  if (child == nullptr || !callback || key.value.empty() || stages.empty() ||
      invalid_repeating_stage) {
    return child;
  }
  return std::make_unique<AnimationElement>(
      std::move(key), std::move(stages), std::move(child),
      std::move(callback));
}

} // namespace cgpui
