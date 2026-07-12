#include "wayland_timer_wakeup_internal.hpp"

#include <poll.h>

int main() {
  cgpui::WaylandTimerWakeup timer;
  if (timer.descriptor() == -1) return 1;
  timer.request(5);
  pollfd descriptor{
      .fd = timer.descriptor(),
      .events = POLLIN,
      .revents = 0,
  };
  if (poll(&descriptor, 1, 500) != 1 ||
      (descriptor.revents & POLLIN) == 0) {
    return 2;
  }
  timer.drain();

  descriptor.revents = 0;
  timer.request(100);
  timer.cancel();
  return poll(&descriptor, 1, 150) == 0 ? 0 : 3;
}
