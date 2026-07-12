#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::sync_platform_time() {
  const std::uint64_t now = application_.monotonic_time_ms();
  if (!platform_clock_initialized_) {
    last_platform_time_ms_ = now;
    platform_clock_initialized_ = true;
    return;
  }
  if (now > last_platform_time_ms_) {
    current_time_ms_ += now - last_platform_time_ms_;
  }
  last_platform_time_ms_ = now;
}

void WindowRuntime::schedule_next_timer_wakeup() {
  if (window_ == nullptr || should_quit_) return;
  const auto timer = std::ranges::min_element(
      timers_, {}, &RuntimeTimer::due_ms);
  if (timer == timers_.end()) {
    application_.cancel_wakeup_after();
    return;
  }
  if (timer->due_ms <= current_time_ms_) {
    request_platform_wakeup();
  } else {
    application_.request_wakeup_after(timer->due_ms - current_time_ms_);
  }
}

} // namespace cgpui
