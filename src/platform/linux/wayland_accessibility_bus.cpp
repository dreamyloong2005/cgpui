#include "wayland_accessibility_internal.hpp"

namespace cgpui {

bool WaylandAtspiAccessibilityAdapter::ensure_dbus_connection(
    WaylandAtspiBusOperations bus_operations,
    WaylandAtspiDbusOperations dbus_operations) {
  if (!automatic_connection_enabled_) return dbus_attached_;
  if (dbus_attached_ && bus_connection_.is_connected()) return true;
  if (dbus_attached_) {
    event_publisher_.detach();
    dbus_registry_.detach();
    dbus_attached_ = false;
  }
  bus_connection_.disconnect();
  if (!bus_connection_.connect(bus_operations)) return false;
  dbus_registry_.attach(bus_connection_.connection(), dbus_operations);
  event_publisher_.attach(bus_connection_.connection(), dbus_operations);
  dbus_registry_.synchronize(atspi_object_nodes_);
  dbus_attached_ = true;
  return true;
}

void WaylandAtspiAccessibilityAdapter::attach_dbus(
    DBusConnection* connection, WaylandAtspiDbusOperations operations) {
  detach_dbus();
  automatic_connection_enabled_ = false;
  if (connection == nullptr) return;
  dbus_registry_.attach(connection, operations);
  event_publisher_.attach(connection, operations);
  dbus_registry_.synchronize(atspi_object_nodes_);
  dbus_attached_ = true;
}

void WaylandAtspiAccessibilityAdapter::detach_dbus() {
  event_publisher_.detach();
  dbus_registry_.detach();
  bus_connection_.disconnect();
  dbus_attached_ = false;
}

WaylandAtspiBusDiagnostics
WaylandAtspiAccessibilityAdapter::bus_diagnostics() const {
  return bus_connection_.diagnostics();
}

WaylandAtspiDbusDiagnostics
WaylandAtspiAccessibilityAdapter::dbus_diagnostics() const {
  return dbus_registry_.diagnostics();
}

bool wayland_atspi_ensure_dbus_connection(
    WaylandAtspiAccessibilityAdapter& adapter) {
  return adapter.ensure_dbus_connection(
      default_wayland_atspi_bus_operations(),
      default_wayland_atspi_dbus_operations());
}

void wayland_atspi_attach_dbus(
    WaylandAtspiAccessibilityAdapter& adapter,
    DBusConnection* connection,
    WaylandAtspiDbusOperations operations) {
  adapter.attach_dbus(connection, operations);
}

void wayland_atspi_detach_dbus(WaylandAtspiAccessibilityAdapter& adapter) {
  adapter.detach_dbus();
}

WaylandAtspiBusDiagnostics wayland_atspi_bus_diagnostics(
    const WaylandAtspiAccessibilityAdapter& adapter) {
  return adapter.bus_diagnostics();
}

WaylandAtspiDbusDiagnostics wayland_atspi_dbus_diagnostics(
    const WaylandAtspiAccessibilityAdapter& adapter) {
  return adapter.dbus_diagnostics();
}

} // namespace cgpui
