#include "wayland_accessibility_internal.hpp"
#include "wayland_atspi_dbus_test_support.hpp"
#include "wayland_services_internal.hpp"

#include <dbus/dbus.h>

#include <string>
#include <vector>

int main() {
  using namespace cgpui;

  test::WaylandAtspiDbusRecorder recorder;
  auto adapter = create_wayland_atspi_accessibility_adapter();
  wayland_atspi_attach_dbus(
      *adapter, recorder.connection(), recorder.operations());

  PlatformAccessibilityTreeUpdate update;
  update.root_element_id = 10;
  update.node_count = 2;
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = PlatformAccessibilityRole::generic,
      .name = "window",
      .child_count = 1,
  });
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 11,
      .parent_element_id = 10,
      .role = PlatformAccessibilityRole::button,
      .name = "Run",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(update));

  if (recorder.registered_paths.size() != 2 ||
      recorder.registered_paths[0].path !=
          "/org/a11y/atspi/accessible/10" ||
      recorder.registered_paths[1].path !=
          "/org/a11y/atspi/accessible/11" ||
      recorder.registered_paths[1].vtable == nullptr ||
      recorder.registered_paths[1].vtable->message_function == nullptr) {
    return 1;
  }

  DBusMessage* introspect = test::atspi_method_call(
      "/org/a11y/atspi/accessible/11",
      DBUS_INTERFACE_INTROSPECTABLE,
      "Introspect");
  if (recorder.call(introspect, 1) != DBUS_HANDLER_RESULT_HANDLED ||
      recorder.sent_reply == nullptr ||
      dbus_message_get_type(recorder.sent_reply) !=
          DBUS_MESSAGE_TYPE_METHOD_RETURN) {
    if (introspect != nullptr) dbus_message_unref(introspect);
    return 2;
  }
  dbus_message_unref(introspect);
  const std::string xml = test::reply_string(recorder.sent_reply);
  if (xml.find("org.a11y.atspi.Accessible") == std::string::npos ||
      xml.find("org.freedesktop.DBus.Introspectable") == std::string::npos ||
      xml.find("GetInterfaces") == std::string::npos) {
    return 3;
  }

  DBusMessage* interfaces = test::atspi_method_call(
      "/org/a11y/atspi/accessible/11",
      "org.a11y.atspi.Accessible",
      "GetInterfaces");
  if (recorder.call(interfaces, 1) != DBUS_HANDLER_RESULT_HANDLED) {
    if (interfaces != nullptr) dbus_message_unref(interfaces);
    return 4;
  }
  dbus_message_unref(interfaces);
  if (test::reply_string_array(recorder.sent_reply) !=
      std::vector<std::string>{
          "org.a11y.atspi.Accessible",
          "org.a11y.atspi.Component"}) {
    return 5;
  }

  PlatformAccessibilityTreeUpdate replacement;
  replacement.root_element_id = 10;
  replacement.node_count = 1;
  replacement.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = PlatformAccessibilityRole::generic,
      .name = "window",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(replacement));
  if (recorder.unregistered_paths !=
      std::vector<std::string>{"/org/a11y/atspi/accessible/11"}) {
    return 6;
  }

  const auto diagnostics = wayland_atspi_dbus_diagnostics(*adapter);
  if (diagnostics.registration_attempt_count != 2 ||
      diagnostics.registered_object_count != 1 ||
      diagnostics.unregistration_count != 1 ||
      diagnostics.method_call_count != 2 ||
      diagnostics.failed_registration_count != 0) {
    return 7;
  }

  wayland_atspi_detach_dbus(*adapter);
  if (recorder.unregistered_paths != std::vector<std::string>{
          "/org/a11y/atspi/accessible/11",
          "/org/a11y/atspi/accessible/10"}) {
    return 8;
  }
  return 0;
}
