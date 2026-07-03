#include "clipboard_wayland_internal.hpp"

namespace cgpui {

#if defined(__linux__)
void WaylandClipboard::Connection::start_dispatch_thread() {
  bool expected = false;
  if (!dispatch_running_.compare_exchange_strong(expected, true)) {
    return;
  }
  dispatch_thread_ = std::thread([this] { dispatch_owned_selection_events(); });
}

void WaylandClipboard::Connection::stop_dispatch_thread() {
  dispatch_running_.store(false);
  if (dispatch_thread_.joinable()) {
    dispatch_thread_.join();
  }
}

void WaylandClipboard::Connection::dispatch_owned_selection_events() {
  if (display_ == nullptr) {
    dispatch_running_.store(false);
    return;
  }

  const int display_fd = wl_display_get_fd(display_);
  while (dispatch_running_.load()) {
    std::lock_guard display_lock(display_mutex_);
    while (wl_display_prepare_read(display_) != 0) {
      if (wl_display_dispatch_pending(display_) == -1) {
        dispatch_running_.store(false);
        return;
      }
    }
    (void)wl_display_flush(display_);

    pollfd descriptor{
        .fd = display_fd,
        .events = POLLIN | POLLHUP | POLLERR,
        .revents = 0,
    };
    const int ready = poll(&descriptor, 1, 50);
    if (ready == -1 && errno == EINTR) {
      wl_display_cancel_read(display_);
      continue;
    }
    if (ready <= 0 || (descriptor.revents & POLLIN) == 0) {
      wl_display_cancel_read(display_);
      continue;
    }
    if (wl_display_read_events(display_) == -1) {
      dispatch_running_.store(false);
      return;
    }
    if (wl_display_dispatch_pending(display_) == -1) {
      dispatch_running_.store(false);
      return;
    }
  }
}
#endif

} // namespace cgpui
