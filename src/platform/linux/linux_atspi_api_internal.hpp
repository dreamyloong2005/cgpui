#pragma once

#include "cgpui/platform/platform_accessibility.hpp"

#include <memory>

namespace cgpui {

class WaylandAtspiAccessibilityAdapter;

struct WaylandAtspiAccessibilityAdapterDeleter {
  void operator()(WaylandAtspiAccessibilityAdapter* adapter) const;
};
using WaylandAtspiAccessibilityAdapterPtr = std::unique_ptr<
    WaylandAtspiAccessibilityAdapter,
    WaylandAtspiAccessibilityAdapterDeleter>;
using LinuxAtspiAccessibilityAdapterPtr =
    WaylandAtspiAccessibilityAdapterPtr;

WaylandAtspiAccessibilityAdapterPtr
create_wayland_atspi_accessibility_adapter();
bool wayland_atspi_ensure_dbus_connection(
    WaylandAtspiAccessibilityAdapter& adapter);
void wayland_atspi_update_accessibility_tree(
    WaylandAtspiAccessibilityAdapter& adapter,
    PlatformAccessibilityTreeUpdate update);

LinuxAtspiAccessibilityAdapterPtr create_linux_atspi_accessibility_adapter();
bool linux_atspi_ensure_dbus_connection(
    WaylandAtspiAccessibilityAdapter& adapter);
void linux_atspi_update_accessibility_tree(
    WaylandAtspiAccessibilityAdapter& adapter,
    PlatformAccessibilityTreeUpdate update);

}  // namespace cgpui
