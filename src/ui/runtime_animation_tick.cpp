#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::tick_animation(AnimationId id) {
  const auto animation = std::find_if(
      animations_.begin(),
      animations_.end(),
      [id](const RuntimeAnimation& animation) {
        return animation.id == id;
      });
  if (animation == animations_.end() || animation->complete ||
      animation->last_tick_ms == current_time_ms_) {
    return;
  }

  animation->last_tick_ms = current_time_ms_;
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
        animations_.begin(),
        animations_.end(),
        [id](const RuntimeAnimation& animation) {
          return animation.id == id;
        });
    if (completed != animations_.end()) {
      completed->complete = true;
      if (completed->timer_id.value != 0) {
        (void)cancel_timer(completed->timer_id);
        completed->timer_id = {};
      }
    }
  }
}

} // namespace cgpui
