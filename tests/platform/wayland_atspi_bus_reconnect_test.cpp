#include "wayland_accessibility_internal.hpp"
#include "wayland_atspi_bus_test_support.hpp"
#include "wayland_atspi_dbus_test_support.hpp"
#include "wayland_services_internal.hpp"

#include <string>
#include <vector>

int main() {
  using namespace cgpui;

  test::WaylandAtspiDbusRecorder dbus_recorder;
  test::WaylandAtspiBusRecorder bus_recorder(dbus_recorder.connection());
  auto adapter = create_wayland_atspi_accessibility_adapter();
  if (!adapter->ensure_dbus_connection(
          bus_recorder.operations(), dbus_recorder.operations())) {
    return 1;
  }

  PlatformAccessibilityTreeUpdate update;
  update.root_element_id = 10;
  update.node_count = 1;
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = PlatformAccessibilityRole::button,
      .name = "Reconnect",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(update));

  bus_recorder.report_disconnect_once();
  bus_recorder.open_succeeds = false;
  if (adapter->ensure_dbus_connection(
          bus_recorder.operations(), dbus_recorder.operations())) {
    return 2;
  }
  const auto failed = adapter->bus_diagnostics();
  if (failed.connection_loss_count != 1 || failed.reconnect_attempt_count != 1 ||
      failed.reconnect_failure_count != 1 || failed.connected ||
      bus_recorder.close_calls != 1 ||
      bus_recorder.accessibility_unref_calls != 1 ||
      dbus_recorder.unregistered_paths !=
          std::vector<std::string>{"/org/a11y/atspi/accessible/10"}) {
    return 3;
  }

  bus_recorder.open_succeeds = true;
  if (!adapter->ensure_dbus_connection(
          bus_recorder.operations(), dbus_recorder.operations())) {
    return 4;
  }
  const auto recovered = adapter->bus_diagnostics();
  if (recovered.discovery_attempt_count != 3 ||
      recovered.connection_attempt_count != 3 ||
      recovered.reconnect_attempt_count != 2 ||
      recovered.reconnect_success_count != 1 ||
      recovered.reconnect_failure_count != 1 || !recovered.connected ||
      bus_recorder.open_calls != 3 || bus_recorder.register_calls != 2 ||
      dbus_recorder.registered_paths.size() != 2 ||
      dbus_recorder.registered_paths.back().path !=
          "/org/a11y/atspi/accessible/10") {
    return 5;
  }

  auto injected = create_wayland_atspi_accessibility_adapter();
  injected->attach_dbus(dbus_recorder.connection(), dbus_recorder.operations());
  const auto discovery_calls = bus_recorder.discovery_calls;
  if (!injected->ensure_dbus_connection(
          bus_recorder.operations(), dbus_recorder.operations()) ||
      bus_recorder.discovery_calls != discovery_calls) {
    return 6;
  }
  return 0;
}
