#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::tick_animation(AnimationId id) {
  const auto animation = std::find_if(
      state_->animations.animations_.begin(),
      state_->animations.animations_.end(),
      [id](const RuntimeAnimation& animation) {
        return animation.id == id;
      });
  if (animation == state_->animations.animations_.end() || animation->complete ||
      animation->cancelled ||
      animation->last_tick_ms == state_->scheduling.current_time_ms_) {
    return;
  }

  animation->last_tick_ms = state_->scheduling.current_time_ms_;
  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  if (!snapshot.has_value()) {
    return;
  }

  AnimationCallback callback = animation->callback;
  if (callback) {
    callback(context(), *snapshot);
  }

  if (snapshot->complete) {
    const auto completed = std::find_if(
        state_->animations.animations_.begin(),
        state_->animations.animations_.end(),
        [id](const RuntimeAnimation& animation) {
          return animation.id == id;
        });
    if (completed != state_->animations.animations_.end()) {
      completed->complete = true;
      completed->frame_pending = false;
    }
  }
}

} // namespace cgpui
