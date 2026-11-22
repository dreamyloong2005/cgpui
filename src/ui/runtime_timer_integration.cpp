#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::sync_platform_time() {
  const std::uint64_t now = state_->core.application_.monotonic_time_ms();
  if (!state_->scheduling.platform_clock_initialized_) {
    state_->scheduling.last_platform_time_ms_ = now;
    state_->scheduling.platform_clock_initialized_ = true;
    return;
  }
  if (now > state_->scheduling.last_platform_time_ms_) {
    state_->scheduling.current_time_ms_ += now - state_->scheduling.last_platform_time_ms_;
  }
  state_->scheduling.last_platform_time_ms_ = now;
}

void WindowRuntime::schedule_next_timer_wakeup() {
  if (state_->windows.window_ == nullptr || state_->core.should_quit_) return;
  const auto timer = std::ranges::min_element(
      state_->scheduling.timers_, {}, &RuntimeTimer::due_ms);
  if (timer == state_->scheduling.timers_.end()) {
    state_->core.application_.cancel_wakeup_after();
    return;
  }
  if (timer->due_ms <= state_->scheduling.current_time_ms_) {
    request_platform_wakeup();
  } else {
    state_->core.application_.request_wakeup_after(timer->due_ms - state_->scheduling.current_time_ms_);
  }
}

} // namespace cgpui
