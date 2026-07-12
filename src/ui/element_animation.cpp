#include "cgpui/ui/element_animation.hpp"

#include <utility>

namespace cgpui {

AnimationElement::AnimationElement(
    ElementKey key,
    AnimationOptions options,
    AnyElement child,
    ElementAnimationCallback callback)
    : animation_key_(std::move(key)),
      options_(options),
      child_(std::move(child)),
      callback_(std::move(callback)) {
  set_key(animation_key_);
  if (child_ != nullptr) {
    set_enabled(child_->enabled());
    set_focus_metadata(child_->focus_metadata());
    set_flex_grow(child_->flex_grow());
    set_flex_shrink(child_->flex_shrink());
    set_position(child_->position());
    set_inset(child_->inset());
  }
}

Element* AnimationElement::child() {
  return child_.get();
}

const Element* AnimationElement::child() const {
  return child_.get();
}

LayoutOutput AnimationElement::layout(LayoutInput input) const {
  if (child_ == nullptr) return Element::layout(input);
  if (input.animation_state_store != nullptr && callback_) {
    const ElementAnimationSnapshot snapshot =
        input.animation_state_store->resolve(animation_key_, options_);
    callback_(*child_, snapshot);
  }
  const LayoutOutput output = child_->layout(input);
  child_->set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  return output;
}

AnyElement with_animation(
    ElementKey key,
    AnimationOptions options,
    AnyElement child,
    ElementAnimationCallback callback) {
  if (child == nullptr || !callback || key.value.empty()) return child;
  return std::make_unique<AnimationElement>(
      std::move(key), options, std::move(child), std::move(callback));
}

} // namespace cgpui
