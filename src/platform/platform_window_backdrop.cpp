#include "cgpui/platform/platform_window.hpp"

namespace cgpui {

PlatformWindowBackdropState PlatformWindow::apply_window_backdrop(
    WindowBackdrop backdrop, bool transparent_surface) {
  return PlatformWindowBackdropState{
      .supported = false,
      .transparent_surface = transparent_surface,
      .backend = "unsupported",
      .requested = backdrop,
      .reason = "window backdrop material unsupported on this backend",
  };
}

PlatformWindowBackdropState PlatformWindow::window_backdrop_state() const {
  return PlatformWindowBackdropState{
      .supported = false,
      .backend = "unsupported",
      .reason = "window backdrop material unsupported on this backend",
  };
}

} // namespace cgpui
