#pragma once

#include "cgpui/platform/platform_accessibility.hpp"
#include "wayland_atspi_dbus_internal.hpp"
#include "wayland_atspi_events_internal.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace cgpui {

class WaylandAtspiAccessibilityAdapter {
 public:
  void update(PlatformAccessibilityTreeUpdate update);
  void attach_dbus(
      DBusConnection* connection,
      WaylandAtspiDbusOperations operations =
          default_wayland_atspi_dbus_operations());
  void detach_dbus();
  [[nodiscard]] WaylandAtspiDbusDiagnostics dbus_diagnostics() const;
  [[nodiscard]] WaylandAtspiEventDiagnostics event_diagnostics() const;
  [[nodiscard]] const std::vector<WaylandAtspiObjectNode>&
  atspi_object_nodes() const;
  [[nodiscard]] const std::vector<PlatformAccessibilityLiveUpdate>&
  last_live_updates() const;

 private:
  PlatformAccessibilityTreeUpdate last_update_;
  std::vector<WaylandAtspiObjectNode> atspi_object_nodes_;
  std::vector<PlatformAccessibilityLiveUpdate> live_updates_;
  WaylandAtspiDbusRegistry dbus_registry_;
  WaylandAtspiEventPublisher event_publisher_;
  std::uint64_t root_element_id_ = 0;
  std::size_t node_count_ = 0;
  std::size_t focused_node_count_ = 0;
  std::size_t text_input_node_count_ = 0;
};

void wayland_atspi_attach_dbus(
    WaylandAtspiAccessibilityAdapter& adapter,
    DBusConnection* connection,
    WaylandAtspiDbusOperations operations);
void wayland_atspi_detach_dbus(WaylandAtspiAccessibilityAdapter& adapter);
[[nodiscard]] WaylandAtspiDbusDiagnostics wayland_atspi_dbus_diagnostics(
    const WaylandAtspiAccessibilityAdapter& adapter);
[[nodiscard]] WaylandAtspiEventDiagnostics wayland_atspi_event_diagnostics(
    const WaylandAtspiAccessibilityAdapter& adapter);

} // namespace cgpui
