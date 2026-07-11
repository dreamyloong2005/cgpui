#include "wayland_application_internal.hpp"

namespace cgpui {

int WaylandApplication::run() {
  return wayland_run_event_loop(
      display_,
      wakeup_pipe_[0],
      running_,
      windows_);
}

void WaylandApplication::request_wakeup() {
  wayland_request_wakeup(wakeup_pipe_[1]);
}

void WaylandApplication::quit() {
  running_.store(false);
  request_wakeup();
}

PlatformReopenResult WaylandApplication::request_reopen() {
  return dispatch_reopen("wayland");
}

} // namespace cgpui
