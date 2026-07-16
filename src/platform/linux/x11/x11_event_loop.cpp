#include "x11_application_internal.hpp"
#include "x11_window_internal.hpp"

#include <cerrno>
#include <climits>
#include <poll.h>
#include <unistd.h>

namespace cgpui {

void X11Application::dispatch_x11_events() {
  while (xcb_generic_event_t* event = xcb_poll_for_event(connection_)) {
    for (X11Window* window : windows_) {
      if (window != nullptr && window->owns_event(*event)) {
        window->handle_event(*event);
        break;
      }
    }
    std::free(event);
  }
}

void X11Application::dispatch_wakeup() {
  if (!wakeup_pending_.exchange(false) && !timer_due()) return;
  {
    std::scoped_lock lock(timer_mutex_);
    if (timer_deadline_ && *timer_deadline_ <= std::chrono::steady_clock::now()) {
      timer_deadline_.reset();
    }
  }
  for (X11Window* window : windows_) {
    if (window != nullptr) window->wakeup_requested();
  }
}

bool X11Application::timer_due() const {
  std::scoped_lock lock(timer_mutex_);
  return timer_deadline_ && *timer_deadline_ <= std::chrono::steady_clock::now();
}

int X11Application::poll_timeout_ms() const {
  std::scoped_lock lock(timer_mutex_);
  if (!timer_deadline_) return -1;
  const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
      *timer_deadline_ - std::chrono::steady_clock::now());
  if (remaining.count() <= 0) return 0;
  return static_cast<int>(std::min<std::int64_t>(remaining.count(), INT_MAX));
}

int X11Application::run() {
  pollfd descriptors[] = {
      {.fd = xcb_get_file_descriptor(connection_), .events = POLLIN},
      {.fd = wakeup_pipe_[0], .events = POLLIN},
  };
  while (running_.load()) {
    const int result = poll(descriptors, 2, poll_timeout_ms());
    if (result < 0 && errno != EINTR) return 1;
    if ((descriptors[1].revents & POLLIN) != 0) {
      char buffer[64];
      while (read(wakeup_pipe_[0], buffer, sizeof(buffer)) > 0) {}
    }
    if ((descriptors[0].revents & POLLIN) != 0) dispatch_x11_events();
    dispatch_wakeup();
    if (xcb_connection_has_error(connection_) != 0) return 2;
  }
  return 0;
}

void X11Application::request_wakeup() {
  if (wakeup_pending_.exchange(true)) return;
  const char value = 1;
  (void)write(wakeup_pipe_[1], &value, 1);
}

void X11Application::request_wakeup_after(std::uint64_t delay_ms) {
  {
    std::scoped_lock lock(timer_mutex_);
    timer_deadline_ = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(delay_ms);
  }
  const char value = 1;
  (void)write(wakeup_pipe_[1], &value, 1);
}

void X11Application::cancel_wakeup_after() {
  std::scoped_lock lock(timer_mutex_);
  timer_deadline_.reset();
}

void X11Application::quit() {
  running_.store(false);
  const char value = 1;
  (void)write(wakeup_pipe_[1], &value, 1);
}

}  // namespace cgpui
