#include "win32_timer_wakeup_internal.hpp"

#include <limits>

namespace cgpui {

Win32TimerWakeup::Win32TimerWakeup()
    : timer_(CreateThreadpoolTimer(&timer_callback, this, nullptr)) {}

Win32TimerWakeup::~Win32TimerWakeup() {
  if (timer_ == nullptr) return;
  SetThreadpoolTimer(timer_, nullptr, 0, 0);
  WaitForThreadpoolTimerCallbacks(timer_, TRUE);
  CloseThreadpoolTimer(timer_);
}

void Win32TimerWakeup::set_thread_id(DWORD thread_id) {
  thread_id_.store(thread_id);
  if (thread_id != 0 && pending_.exchange(false)) {
    PostThreadMessageW(thread_id, cgpui_wakeup_message, 0, 0);
  }
}

void Win32TimerWakeup::request_now() { post_wakeup(); }

void Win32TimerWakeup::request(std::uint64_t delay_ms) {
  if (timer_ == nullptr) {
    post_wakeup();
    return;
  }
  const std::uint64_t maximum_delay =
      (std::numeric_limits<std::uint64_t>::max)() / 10'000;
  std::uint64_t bounded_delay = delay_ms == 0 ? 1 : delay_ms;
  if (bounded_delay > maximum_delay) bounded_delay = maximum_delay;
  const std::uint64_t ticks = bounded_delay * 10'000;
  ULARGE_INTEGER due_ticks{};
  due_ticks.QuadPart = 0 - ticks;
  FILETIME due{
      .dwLowDateTime = due_ticks.LowPart,
      .dwHighDateTime = due_ticks.HighPart,
  };
  SetThreadpoolTimer(timer_, &due, 0, 0);
}

void Win32TimerWakeup::cancel() {
  pending_.store(false);
  if (timer_ != nullptr) SetThreadpoolTimer(timer_, nullptr, 0, 0);
}

void CALLBACK Win32TimerWakeup::timer_callback(
    PTP_CALLBACK_INSTANCE,
    void* context,
    PTP_TIMER) {
  static_cast<Win32TimerWakeup*>(context)->post_wakeup();
}

void Win32TimerWakeup::post_wakeup() {
  DWORD thread_id = thread_id_.load();
  if (thread_id == 0) {
    pending_.store(true);
    thread_id = thread_id_.load();
    if (thread_id == 0 || !pending_.exchange(false)) return;
  }
  PostThreadMessageW(thread_id, cgpui_wakeup_message, 0, 0);
}

} // namespace cgpui
