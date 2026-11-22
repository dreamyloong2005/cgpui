#include "ui_internal.hpp"

namespace cgpui {

TimerId WindowRuntime::schedule_window_timer(
    WindowRuntimeId id, std::uint64_t delay, bool repeating, TimerCallback callback) {
  id = normalize_window_context_id(id);
  if (id != state_->windows.root_window_runtime_id_) {
    const auto* record = find_window_runtime_record(id);
    if (record == nullptr || !record->active) return {};
  }
  callback = bind_window_callback(id, std::move(callback));
  const TimerId timer_id = repeating ? schedule_repeating_timer(delay, std::move(callback))
                                    : schedule_timer(delay, std::move(callback));
  const auto timer = std::find_if(state_->scheduling.timers_.begin(), state_->scheduling.timers_.end(),
      [timer_id](const auto& value) { return value.id == timer_id; });
  if (timer != state_->scheduling.timers_.end()) timer->window_runtime_id = id;
  return timer_id;
}

void WindowRuntime::cancel_window_timers(WindowRuntimeId id) {
  // Retire captures after the timer container is coherent: their destructors
  // may schedule or cancel other work on this runtime.
  std::vector<TimerCallback> retired;
  for (auto& timer : state_->scheduling.timers_) {
    if (timer.window_runtime_id == id) retired.push_back(std::move(timer.callback));
  }
  std::erase_if(state_->scheduling.timers_, [id](const auto& timer) { return timer.window_runtime_id == id; });
  schedule_next_timer_wakeup();
}

} // namespace cgpui
