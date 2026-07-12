#include "wayland_accessibility_internal.hpp"
#include "wayland_atspi_event_test_support.hpp"
#include "wayland_atspi_events_internal.hpp"
#include "wayland_services_internal.hpp"

#include <dbus/dbus.h>

#include <string>

int main() {
  using namespace cgpui;

  test::WaylandAtspiDbusRecorder recorder;
  auto adapter = create_wayland_atspi_accessibility_adapter();
  wayland_atspi_attach_dbus(
      *adapter, recorder.connection(), recorder.operations());

  PlatformAccessibilityTreeUpdate initial;
  initial.root_element_id = 10;
  initial.node_count = 1;
  initial.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = PlatformAccessibilityRole::text_input,
      .name = "Editor",
      .text = "old",
      .value = "old",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(initial));
  recorder.clear_messages();

  PlatformAccessibilityTreeUpdate update;
  update.root_element_id = 10;
  update.node_count = 1;
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = PlatformAccessibilityRole::text_input,
      .name = "Editor",
      .text = "new text",
      .value = "42",
  });
  update.live_updates.push_back(PlatformAccessibilityLiveUpdate{
      .kind = PlatformAccessibilityLiveUpdateKind::value_changed,
      .element_id = 10,
      .value = "42",
  });
  update.live_updates.push_back(PlatformAccessibilityLiveUpdate{
      .kind = PlatformAccessibilityLiveUpdateKind::text_changed,
      .element_id = 10,
      .text = "A\xC3\xA9",
  });
  update.live_updates.push_back(PlatformAccessibilityLiveUpdate{
      .kind = PlatformAccessibilityLiveUpdateKind::value_changed,
      .element_id = 99,
      .value = "missing",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(update));

  if (recorder.sent_messages.size() != 2) return 1;
  DBusMessage* value_event = recorder.sent_messages[0];
  if (std::string(dbus_message_get_path(value_event)) !=
          "/org/a11y/atspi/accessible/10" ||
      std::string(dbus_message_get_interface(value_event)) !=
          "org.a11y.atspi.Event.Object" ||
      std::string(dbus_message_get_member(value_event)) != "PropertyChange" ||
      std::string(dbus_message_get_signature(value_event)) != "siiv(so)") {
    return 2;
  }
  const auto value = test::read_atspi_event_payload(value_event);
  if (value.detail != "accessible-value" || value.value != "42" ||
      value.source != test::AtspiObjectReference{
          ":1.88", "/org/a11y/atspi/accessible/10"}) {
    return 3;
  }

  DBusMessage* text_event = recorder.sent_messages[1];
  if (std::string(dbus_message_get_member(text_event)) != "TextChanged" ||
      std::string(dbus_message_get_signature(text_event)) != "siiv(so)") {
    return 4;
  }
  const auto text = test::read_atspi_event_payload(text_event);
  if (text.detail != "insert" || text.detail1 != 0 || text.detail2 != 2 ||
      text.value != "A\xC3\xA9") {
    return 5;
  }

  recorder.clear_messages();
  recorder.send_result = false;
  PlatformAccessibilityTreeUpdate failed_send;
  failed_send.root_element_id = 10;
  failed_send.node_count = 1;
  failed_send.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = PlatformAccessibilityRole::text_input,
      .name = "Editor",
      .text = "new text",
      .value = "43",
  });
  failed_send.live_updates.push_back(PlatformAccessibilityLiveUpdate{
      .kind = PlatformAccessibilityLiveUpdateKind::value_changed,
      .element_id = 10,
      .value = "43",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(failed_send));
  if (recorder.sent_messages.size() != 1) return 6;

  recorder.clear_messages();
  recorder.send_result = true;
  wayland_atspi_detach_dbus(*adapter);
  PlatformAccessibilityTreeUpdate disconnected;
  disconnected.root_element_id = 10;
  disconnected.node_count = 1;
  disconnected.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = PlatformAccessibilityRole::text_input,
      .name = "Editor",
      .text = "new text",
      .value = "44",
  });
  disconnected.live_updates.push_back(PlatformAccessibilityLiveUpdate{
      .kind = PlatformAccessibilityLiveUpdateKind::value_changed,
      .element_id = 10,
      .value = "44",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(disconnected));
  if (!recorder.sent_messages.empty()) return 7;

  const auto diagnostics = wayland_atspi_event_diagnostics(*adapter);
  if (diagnostics.value_event_count != 1 ||
      diagnostics.text_event_count != 1 ||
      diagnostics.missing_object_count != 1 ||
      diagnostics.no_connection_count != 1 ||
      diagnostics.send_failure_count != 1) {
    return 8;
  }
  return 0;
}
