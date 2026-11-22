#include "ui_internal.hpp"

namespace cgpui {

AnimationHandle WindowRuntime::start_animation(
    AnimationOptions options,
    AnimationCallback callback) {
  if (!callback || state_->core.shutting_down_) {
    return {};
  }

  if (options.tick_interval_ms == 0) {
    options.tick_interval_ms = 16;
  }

  const AnimationId id{state_->animations.next_animation_id_++};
  state_->animations.animations_.push_back(RuntimeAnimation{
      .id = id,
      .options = options,
      .callback = std::move(callback),
      .started_ms = state_->scheduling.current_time_ms_,
      .last_tick_ms = state_->scheduling.current_time_ms_,
      .next_tick_ms = next_animation_frame_deadline(0, options.tick_interval_ms),
      .complete = options.duration_ms == 0,
      .cancelled = false,
      .frame_pending = options.duration_ms != 0,
  });

  if (options.duration_ms == 0) {
    const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
    AnimationCallback immediate_callback = state_->animations.animations_.back().callback;
    if (snapshot.has_value() && immediate_callback) {
      immediate_callback(context(), *snapshot);
    }
    const auto completed = std::find_if(
        state_->animations.animations_.begin(), state_->animations.animations_.end(),
        [id](const RuntimeAnimation& animation) { return animation.id == id; });
    if (completed != state_->animations.animations_.end()) completed->callback = {};
  } else {
    schedule_animation_frame_wakeup(true);
  }

  return AnimationHandle(*this, id);
}

} // namespace cgpui
