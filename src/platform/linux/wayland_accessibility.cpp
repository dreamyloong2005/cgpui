#include "wayland_accessibility_internal.hpp"
#include "wayland_services_internal.hpp"

#include <utility>

namespace cgpui {

void WaylandAtspiAccessibilityAdapter::update(
    PlatformAccessibilityTreeUpdate update) {
  last_update_ = std::move(update);
  live_updates_ = last_update_.live_updates;
  root_element_id_ = last_update_.root_element_id;
  node_count_ = last_update_.node_count;
  focused_node_count_ = last_update_.focused_node_count;
  text_input_node_count_ = 0;
  for (const PlatformAccessibilityNodeUpdate& node : last_update_.nodes) {
    if (node.role == PlatformAccessibilityRole::text_input) {
      text_input_node_count_ += 1;
    }
  }
  atspi_object_nodes_ = build_wayland_atspi_object_nodes(last_update_);
  dbus_registry_.synchronize(atspi_object_nodes_);
}

void WaylandAtspiAccessibilityAdapter::attach_dbus(
    DBusConnection* connection, WaylandAtspiDbusOperations operations) {
  dbus_registry_.attach(connection, operations);
  dbus_registry_.synchronize(atspi_object_nodes_);
}

void WaylandAtspiAccessibilityAdapter::detach_dbus() {
  dbus_registry_.detach();
}

WaylandAtspiDbusDiagnostics
WaylandAtspiAccessibilityAdapter::dbus_diagnostics() const {
  return dbus_registry_.diagnostics();
}

const std::vector<WaylandAtspiObjectNode>&
WaylandAtspiAccessibilityAdapter::atspi_object_nodes() const {
  return atspi_object_nodes_;
}

const std::vector<PlatformAccessibilityLiveUpdate>&
WaylandAtspiAccessibilityAdapter::last_live_updates() const {
  return live_updates_;
}

void WaylandAtspiAccessibilityAdapterDeleter::operator()(
    WaylandAtspiAccessibilityAdapter* adapter) const {
  delete adapter;
}

WaylandAtspiAccessibilityAdapterPtr
create_wayland_atspi_accessibility_adapter() {
  return WaylandAtspiAccessibilityAdapterPtr(
      new WaylandAtspiAccessibilityAdapter());
}

void wayland_atspi_update_accessibility_tree(
    WaylandAtspiAccessibilityAdapter& adapter,
    PlatformAccessibilityTreeUpdate update) {
  adapter.update(std::move(update));
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

WaylandAtspiDbusDiagnostics wayland_atspi_dbus_diagnostics(
    const WaylandAtspiAccessibilityAdapter& adapter) {
  return adapter.dbus_diagnostics();
}

} // namespace cgpui
