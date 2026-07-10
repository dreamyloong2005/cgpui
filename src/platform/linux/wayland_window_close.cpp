#include "wayland_window_internal.hpp"

namespace cgpui {

void WaylandWindow::request_close() {
  close_requested(WindowCloseRequestSource::application);
}

void WaylandWindow::close_requested(WindowCloseRequestSource source) {
  if (!close_controller_.begin(source)) {
    return;
  }
  state_.close_requested = true;
  callback_(close_controller_.event());
}

PlatformWindowCloseState WaylandWindow::close_request_state() const {
  return close_controller_.state();
}

bool WaylandWindow::resolve_close_request(
    PlatformWindowCloseResolution resolution) {
  if (!close_controller_.resolve(resolution)) {
    return false;
  }
  if (resolution == PlatformWindowCloseResolution::cancel) {
    state_.close_requested = false;
  }
  return true;
}

} // namespace cgpui
