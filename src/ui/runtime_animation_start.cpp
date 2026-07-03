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
      .complete = options.duration_ms == 0,
  });

  if (options.duration_ms != 0) {
    const TimerId timer_id = schedule_repeating_timer(
        options.tick_interval_ms,
        [this, id](const WindowRuntimeContext&) {
          tick_animation(id);
        });
    auto stored = std::find_if(
        animations_.begin(),
        animations_.end(),
        [id](const RuntimeAnimation& animation) {
          return animation.id == id;
        });
    if (stored != animations_.end()) {
      stored->timer_id = timer_id;
    }
  }

  return AnimationHandle(*this, id);
}

} // namespace cgpui
