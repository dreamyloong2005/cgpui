#include "wayland_timer_wakeup_internal.hpp"

#include <cerrno>
#include <sys/timerfd.h>
#include <unistd.h>

namespace cgpui {

WaylandTimerWakeup::WaylandTimerWakeup()
    : descriptor_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)) {}

WaylandTimerWakeup::~WaylandTimerWakeup() {
  if (descriptor_ != -1) close(descriptor_);
}

void WaylandTimerWakeup::request(std::uint64_t delay_ms) {
  if (descriptor_ == -1) return;
  itimerspec timer{
      .it_interval = {},
      .it_value = {
          .tv_sec = static_cast<time_t>(delay_ms / 1'000),
          .tv_nsec = static_cast<long>((delay_ms % 1'000) * 1'000'000),
      },
  };
  if (delay_ms == 0) timer.it_value.tv_nsec = 1;
  (void)timerfd_settime(descriptor_, 0, &timer, nullptr);
}

void WaylandTimerWakeup::cancel() {
  if (descriptor_ == -1) return;
  const itimerspec timer{};
  (void)timerfd_settime(descriptor_, 0, &timer, nullptr);
}

void WaylandTimerWakeup::drain() {
  if (descriptor_ == -1) return;
  std::uint64_t expirations = 0;
  while (read(descriptor_, &expirations, sizeof(expirations)) == -1 &&
         errno == EINTR) {}
}

} // namespace cgpui
