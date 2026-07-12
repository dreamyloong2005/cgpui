#include "ui_internal.hpp"

namespace cgpui {

TimerId WindowRuntime::schedule_timer(
    std::uint64_t delay_ms,
    TimerCallback callback) {
  if (!callback) {
    return {};
  }
  const TimerId id{next_timer_id_++};
  timers_.push_back(RuntimeTimer{
      .id = id,
      .due_ms = current_time_ms_ + delay_ms,
      .interval_ms = 0,
      .repeating = false,
      .callback = std::move(callback),
  });
  request_platform_wakeup();
  return id;
}

TimerId WindowRuntime::schedule_repeating_timer(
    std::uint64_t interval_ms,
    TimerCallback callback) {
  if (!callback || interval_ms == 0) {
    return {};
  }
  const TimerId id{next_timer_id_++};
  timers_.push_back(RuntimeTimer{
      .id = id,
      .due_ms = current_time_ms_ + interval_ms,
      .interval_ms = interval_ms,
      .repeating = true,
      .callback = std::move(callback),
  });
  request_platform_wakeup();
  return id;
}

bool WindowRuntime::cancel_timer(TimerId id) {
  if (id.value == 0) {
    return false;
  }
  const auto timer = std::find_if(
      timers_.begin(),
      timers_.end(),
      [id](const RuntimeTimer& timer) {
        return timer.id == id;
      });
  if (timer == timers_.end()) {
    return false;
  }
  timers_.erase(timer);
  return true;
}

void WindowRuntime::advance_time(std::uint64_t delta_ms) {
  current_time_ms_ += delta_ms;
  fire_due_timers();
  flush_deferred_redraw_request();
}

void WindowRuntime::fire_due_timers() {
  if (firing_timers_ || should_quit_) {
    return;
  }

  firing_timers_ = true;
  while (!should_quit_) {
    auto timer = std::find_if(
        timers_.begin(),
        timers_.end(),
        [this](const RuntimeTimer& timer) {
          return timer.callback && timer.due_ms <= current_time_ms_;
        });
    if (timer == timers_.end()) {
      break;
    }

    const TimerId fired_id = timer->id;
    TimerCallback callback = timer->callback;
    if (timer->repeating) {
      timer->due_ms += timer->interval_ms;
    } else {
      timer = timers_.erase(timer);
    }
    record_platform_diagnostic(PlatformDiagnosticEvent{
        .kind = PlatformDiagnosticKind::scheduling,
        .backend = "runtime",
        .operation = "timer-fired",
        .supported = true,
        .succeeded = true,
        .value_count = static_cast<std::size_t>(fired_id.value),
    });
    if (callback) {
      callback(context());
    }
  }
  firing_timers_ = false;
}

} // namespace cgpui
