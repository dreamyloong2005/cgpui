#include "wayland_accessibility_internal.hpp"
#include "wayland_atspi_bus_internal.hpp"
#include "wayland_atspi_bus_test_support.hpp"
#include "wayland_atspi_dbus_test_support.hpp"
#include "wayland_services_internal.hpp"

#include <dbus/dbus.h>

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
      .name = "Save",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(update));

  const auto diagnostics = adapter->bus_diagnostics();
  if (bus_recorder.discovery_calls != 1 || bus_recorder.open_calls != 1 ||
      bus_recorder.opened_address != "unix:path=/tmp/cgpui-atspi" ||
      !bus_recorder.disabled_exit_on_disconnect ||
      diagnostics.discovery_attempt_count != 1 ||
      diagnostics.connection_attempt_count != 1 || !diagnostics.connected ||
      dbus_recorder.registered_paths.size() != 1) {
    return 2;
  }

  wayland_atspi_detach_dbus(*adapter);
  if (bus_recorder.close_calls != 1 ||
      bus_recorder.session_unref_calls != 1 ||
      bus_recorder.accessibility_unref_calls != 1 ||
      adapter->bus_diagnostics().disconnect_count != 1) {
    return 3;
  }

  test::WaylandAtspiBusRecorder failed_discovery(dbus_recorder.connection());
  failed_discovery.return_address = false;
  WaylandAtspiBusConnection discovery_failure;
  if (discovery_failure.connect(failed_discovery.operations()) ||
      discovery_failure.diagnostics().discovery_failure_count != 1 ||
      failed_discovery.open_calls != 0) {
    return 4;
  }

  test::WaylandAtspiBusRecorder failed_open(dbus_recorder.connection());
  failed_open.open_succeeds = false;
  WaylandAtspiBusConnection connection_failure;
  if (connection_failure.connect(failed_open.operations()) ||
      connection_failure.diagnostics().connection_failure_count != 1 ||
      failed_open.open_calls != 1) {
    return 5;
  }

  test::WaylandAtspiBusRecorder failed_register(dbus_recorder.connection());
  failed_register.register_succeeds = false;
  WaylandAtspiBusConnection registration_failure;
  if (registration_failure.connect(failed_register.operations()) ||
      registration_failure.diagnostics().connection_failure_count != 1 ||
      failed_register.close_calls != 1 ||
      failed_register.accessibility_unref_calls != 1) {
    return 6;
  }
  return 0;
}
