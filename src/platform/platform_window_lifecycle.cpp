#include "cgpui/platform/platform_window.hpp"

namespace cgpui {

PlatformWindowLifecycleState PlatformWindow::lifecycle_state() const {
  return PlatformWindowLifecycleState{
      .close_requested = state().close_requested,
  };
}

} // namespace cgpui
