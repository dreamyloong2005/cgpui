#include "ui_internal.hpp"

namespace cgpui {

bool WindowRuntime::cancel_animation(AnimationId id) {
  if (id.value == 0) return false;
  const auto animation = std::find_if(
      animations_.begin(), animations_.end(),
      [id](const RuntimeAnimation& item) { return item.id == id; });
  if (animation == animations_.end()) return false;

  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  if (animation->complete || animation->cancelled || !snapshot.has_value() ||
      snapshot->complete) return false;

  const bool timer_was_active = animation->frame_pending;
  animation->cancelled_elapsed_ms = snapshot->elapsed_ms;
  animation->cancelled = true;
  animation->frame_pending = false;
  animation->callback = {};
  last_animation_cancellation_ = AnimationCancellationDiagnostic{
      .id = id,
      .elapsed_ms = snapshot->elapsed_ms,
      .duration_ms = snapshot->duration_ms,
      .timer_was_active = timer_was_active,
  };
  schedule_animation_frame_wakeup(false);
  return true;
}

bool WindowRuntime::animation_cancelled(AnimationId id) const {
  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  return snapshot.has_value() && snapshot->cancelled;
}

} // namespace cgpui
