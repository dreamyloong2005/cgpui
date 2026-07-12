#include "ui_internal.hpp"

namespace cgpui {

AnimationHandle WindowRuntime::start_animation(
    AnimationOptions options,
    AnimationCallback callback) {
  if (!callback) {
    return {};
  }

  if (options.tick_interval_ms == 0) {
    options.tick_interval_ms = 16;
  }

  const AnimationId id{next_animation_id_++};
  animations_.push_back(RuntimeAnimation{
      .id = id,
      .options = options,
      .callback = std::move(callback),
      .started_ms = current_time_ms_,
      .last_tick_ms = current_time_ms_,
      .next_tick_ms = next_animation_frame_deadline(0, options.tick_interval_ms),
      .complete = options.duration_ms == 0,
      .cancelled = false,
      .frame_pending = options.duration_ms != 0,
  });

  if (options.duration_ms == 0) {
    const std::optional<AnimationSnapshot> snapshot = animation_snapshot(id);
    AnimationCallback immediate_callback = animations_.back().callback;
    if (snapshot.has_value() && immediate_callback) {
      immediate_callback(context(), *snapshot);
    }
    const auto completed = std::find_if(
        animations_.begin(), animations_.end(),
        [id](const RuntimeAnimation& animation) { return animation.id == id; });
    if (completed != animations_.end()) completed->callback = {};
  } else {
    schedule_animation_frame_wakeup(true);
  }

  return AnimationHandle(*this, id);
}

} // namespace cgpui
