#include "wayland_internal.hpp"
#include "wayland_timer_wakeup_internal.hpp"

namespace cgpui {
namespace {

void drain_wakeup_pipe(int wakeup_read_fd) {
  if (wakeup_read_fd == -1) return;
  std::array<std::uint8_t, 64> buffer{};
  while (true) {
    const ssize_t result = read(
        wakeup_read_fd, buffer.data(), buffer.size());
    if (result > 0) continue;
    if (result == -1 && errno == EINTR) continue;
    return;
  }
}

void dispatch_wakeup(std::vector<WaylandWindow*>& windows) {
  for (WaylandWindow* window : windows) {
    if (window != nullptr) {
      wayland_window_wakeup_requested(*window);
    }
  }
}

} // namespace

int wayland_run_event_loop(
    wl_display* display,
    int wakeup_read_fd,
    WaylandTimerWakeup& timer_wakeup,
    std::atomic_bool& running,
    std::vector<WaylandWindow*>& windows) {
  while (running.load() && display != nullptr) {
    while (wl_display_prepare_read(display) != 0) {
      if (wl_display_dispatch_pending(display) == -1) return 1;
    }
    const int flush_result = wl_display_flush(display);
    const bool flush_pending = flush_result == -1 && errno == EAGAIN;
    if (flush_result == -1 && !flush_pending) {
      wl_display_cancel_read(display);
      return 1;
    }

    std::array<pollfd, 3> fds{
        pollfd{
            .fd = wl_display_get_fd(display),
            .events = static_cast<short>(POLLIN | (flush_pending ? POLLOUT : 0)),
            .revents = 0,
        },
        pollfd{
            .fd = wakeup_read_fd,
            .events = POLLIN,
            .revents = 0,
        },
        pollfd{
            .fd = timer_wakeup.descriptor(),
            .events = POLLIN,
            .revents = 0,
        },
    };
    const int poll_result = poll(fds.data(), fds.size(), -1);
    if (poll_result == -1) {
      wl_display_cancel_read(display);
      if (errno == EINTR) {
        continue;
      }
      return 1;
    }
    if ((fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
      wl_display_cancel_read(display);
      return 1;
    }
    if ((fds[0].revents & POLLIN) != 0) {
      if (wl_display_read_events(display) == -1) return 1;
    } else {
      wl_display_cancel_read(display);
    }
    if ((fds[1].revents & POLLIN) != 0) {
      drain_wakeup_pipe(wakeup_read_fd);
      if (running.load()) dispatch_wakeup(windows);
    }
    if ((fds[2].revents & POLLIN) != 0) {
      timer_wakeup.drain();
      if (running.load()) dispatch_wakeup(windows);
    }
  }
  return 0;
}

void wayland_request_wakeup(int wakeup_write_fd) {
  if (wakeup_write_fd == -1) return;
  const std::uint8_t byte = 1;
  while (write(wakeup_write_fd, &byte, sizeof(byte)) == -1 &&
         errno == EINTR) {}
}

} // namespace cgpui
