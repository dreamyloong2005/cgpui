#pragma once

#include "wayland_atspi_dbus_internal.hpp"

#include <cstddef>
#include <span>

namespace cgpui {

struct WaylandAtspiEventDiagnostics {
  std::size_t value_event_count = 0;
  std::size_t text_event_count = 0;
  std::size_t missing_object_count = 0;
  std::size_t no_connection_count = 0;
  std::size_t focus_event_count = 0;
  std::size_t send_failure_count = 0;
};

class WaylandAtspiEventPublisher {
 public:
  void attach(
      DBusConnection* connection,
      WaylandAtspiDbusOperations operations);
  void detach();
  void publish(
      std::span<const WaylandAtspiObjectNode> objects,
      std::span<const PlatformAccessibilityLiveUpdate> updates);
  [[nodiscard]] WaylandAtspiEventDiagnostics diagnostics() const;

 private:
  DBusConnection* connection_ = nullptr;
  WaylandAtspiDbusOperations operations_;
  WaylandAtspiEventDiagnostics diagnostics_;
};

} // namespace cgpui
