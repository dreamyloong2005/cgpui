#include "wayland_internal.hpp"

namespace cgpui {
namespace {

void drain_wakeup_pipe(int wakeup_read_fd) {
  if (wakeup_read_fd == -1) {
    return;
  }
  std::array<std::uint8_t, 64> buffer{};
  while (read(wakeup_read_fd, buffer.data(), buffer.size()) > 0) {}
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
    bool& running,
    std::vector<WaylandWindow*>& windows) {
  while (running && display != nullptr) {
    const int pending = wl_display_dispatch_pending(display);
    if (pending == -1) {
      return 1;
    }
    if (pending > 0) {
      continue;
    }
    (void)wl_display_flush(display);

    std::array<pollfd, 2> fds{
        pollfd{
            .fd = wl_display_get_fd(display),
            .events = POLLIN,
            .revents = 0,
        },
        pollfd{
            .fd = wakeup_read_fd,
            .events = POLLIN,
            .revents = 0,
        },
    };
    const int poll_result = poll(fds.data(), fds.size(), -1);
    if (poll_result == -1) {
      if (errno == EINTR) {
        continue;
      }
      return 1;
    }
    if ((fds[1].revents & POLLIN) != 0) {
      drain_wakeup_pipe(wakeup_read_fd);
      dispatch_wakeup(windows);
    }
    if ((fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
      return 1;
    }
    if ((fds[0].revents & POLLIN) != 0 &&
        wl_display_dispatch(display) == -1) {
      return 1;
    }
  }
  return 0;
}

void wayland_request_wakeup(int wakeup_write_fd) {
  if (wakeup_write_fd == -1) {
    return;
  }
  const std::uint8_t byte = 1;
  const ssize_t written = write(wakeup_write_fd, &byte, sizeof(byte));
  (void)written;
}

} // namespace cgpui
