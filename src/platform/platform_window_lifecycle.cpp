#include "cgpui/platform/platform_window.hpp"

namespace cgpui {

PlatformWindowLifecycleState PlatformWindow::lifecycle_state() const {
  return PlatformWindowLifecycleState{
      .close_requested = state().close_requested,
  };
}

bool PlatformWindow::request_display_state(
    PlatformWindowDisplayState display_state) {
  (void)display_state;
  return false;
}

} // namespace cgpui
