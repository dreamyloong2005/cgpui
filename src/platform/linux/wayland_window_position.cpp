#include "wayland_window_internal.hpp"

namespace cgpui {

PlatformWindowPositionState WaylandWindow::position_state() const {
  return {};
}

bool WaylandWindow::request_position(Point position) {
  (void)position;
  return false;
}

} // namespace cgpui
