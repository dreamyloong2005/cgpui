#include "cgpui/platform/platform_window.hpp"

namespace cgpui {

PlatformWindowChromeState PlatformWindow::apply_window_chrome(
    WindowChromeOptions options) {
  return PlatformWindowChromeState{
      .supported = false,
      .backend = "unsupported",
      .requested = options,
      .applied = WindowChromeOptions{},
      .reason = "window chrome customization unsupported",
  };
}

} // namespace cgpui
