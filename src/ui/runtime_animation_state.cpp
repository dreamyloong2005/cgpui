#include "ui_internal.hpp"

namespace cgpui {

std::optional<AnimationSnapshot> WindowRuntime::animation_snapshot(
    AnimationId id) const {
  if (id.value == 0) {
    return std::nullopt;
  }

  const auto animation = std::find_if(
      animations_.begin(),
      animations_.end(),
      [id](const RuntimeAnimation& animation) {
        return animation.id == id;
      });
  if (animation == animations_.end()) {
    return std::nullopt;
  }

  const std::uint64_t duration_ms = animation->options.duration_ms;
  const std::uint64_t raw_elapsed_ms =
      current_time_ms_ >= animation->started_ms
          ? current_time_ms_ - animation->started_ms
          : 0;
  const std::uint64_t elapsed_ms =
      duration_ms == 0 ? 0 : std::min(raw_elapsed_ms, duration_ms);
  const float linear_progress =
      duration_ms == 0
          ? 1.0F
          : clamp_animation_progress(
                static_cast<float>(elapsed_ms) /
                static_cast<float>(duration_ms));
  const float eased_progress =
      animation->options.curve.value_at(
          linear_progress, animation->options.easing);

  return AnimationSnapshot{
      .id = animation->id,
      .elapsed_ms = elapsed_ms,
      .duration_ms = duration_ms,
      .linear_progress = linear_progress,
      .eased_progress = eased_progress,
      .easing = animation->options.easing,
      .curve = animation->options.curve,
      .complete = animation->complete || linear_progress >= 1.0F,
  };
}

bool WindowRuntime::cancel_animation(AnimationId id) {
  if (id.value == 0) {
    return false;
  }

  const auto animation = std::find_if(
      animations_.begin(),
      animations_.end(),
      [id](const RuntimeAnimation& animation) {
        return animation.id == id;
      });
  if (animation == animations_.end()) {
    return false;
  }
  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  if (animation->complete || !snapshot.has_value() || snapshot->complete) {
    return false;
  }

  animation->complete = true;
  if (animation->timer_id.value != 0) {
    (void)cancel_timer(animation->timer_id);
    animation->timer_id = {};
  }
  return true;
}

bool WindowRuntime::animation_active(AnimationId id) const {
  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  return snapshot.has_value() && !snapshot->complete;
}

bool WindowRuntime::animation_complete(AnimationId id) const {
  const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
  return snapshot.has_value() && snapshot->complete;
}

} // namespace cgpui
