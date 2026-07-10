#include "cgpui/platform/platform_window.hpp"

namespace cgpui {

PlatformWindowPositionState PlatformWindow::position_state() const {
  return {};
}

bool PlatformWindow::request_position(Point position) {
  (void)position;
  return false;
}

} // namespace cgpui
