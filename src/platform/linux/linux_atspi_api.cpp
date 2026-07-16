#include "linux_atspi_api_internal.hpp"

namespace cgpui {

LinuxAtspiAccessibilityAdapterPtr create_linux_atspi_accessibility_adapter() {
  return create_wayland_atspi_accessibility_adapter();
}

bool linux_atspi_ensure_dbus_connection(
    WaylandAtspiAccessibilityAdapter& adapter) {
  return wayland_atspi_ensure_dbus_connection(adapter);
}

void linux_atspi_update_accessibility_tree(
    WaylandAtspiAccessibilityAdapter& adapter,
    PlatformAccessibilityTreeUpdate update) {
  wayland_atspi_update_accessibility_tree(adapter, std::move(update));
}

}  // namespace cgpui
