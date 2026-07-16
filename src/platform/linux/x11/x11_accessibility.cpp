#include "x11_window_internal.hpp"

namespace cgpui {

void X11Window::update_accessibility_tree(
    PlatformAccessibilityTreeUpdate update) {
  (void)linux_atspi_ensure_dbus_connection(*atspi_accessibility_);
  linux_atspi_update_accessibility_tree(
      *atspi_accessibility_, std::move(update));
}

}  // namespace cgpui
