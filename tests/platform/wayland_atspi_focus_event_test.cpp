#include "wayland_accessibility_internal.hpp"
#include "wayland_atspi_event_test_support.hpp"
#include "wayland_services_internal.hpp"

#include <dbus/dbus.h>

#include <string>

namespace {

cgpui::PlatformAccessibilityTreeUpdate focus_update(bool focused) {
  cgpui::PlatformAccessibilityTreeUpdate update;
  update.root_element_id = 10;
  update.node_count = 1;
  update.focused_node_count = focused ? 1 : 0;
  update.nodes.push_back(cgpui::PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = cgpui::PlatformAccessibilityRole::text_input,
      .name = "Editor",
      .focused = focused,
  });
  update.live_updates.push_back(cgpui::PlatformAccessibilityLiveUpdate{
      .kind = cgpui::PlatformAccessibilityLiveUpdateKind::focus_changed,
      .element_id = 10,
      .focused = focused,
  });
  return update;
}

} // namespace

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
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(initial));
  recorder.clear_messages();

  wayland_atspi_update_accessibility_tree(*adapter, focus_update(true));
  wayland_atspi_update_accessibility_tree(*adapter, focus_update(false));

  if (recorder.sent_messages.size() != 2) return 1;
  for (std::size_t index = 0; index < recorder.sent_messages.size(); ++index) {
    DBusMessage* message = recorder.sent_messages[index];
    if (std::string(dbus_message_get_path(message)) !=
            "/org/a11y/atspi/accessible/10" ||
        std::string(dbus_message_get_interface(message)) !=
            "org.a11y.atspi.Event.Object" ||
        std::string(dbus_message_get_member(message)) != "StateChanged" ||
        std::string(dbus_message_get_signature(message)) != "siiv(so)") {
      return 2;
    }
    const test::AtspiEventPayload payload = test::read_atspi_event_payload(message);
    if (payload.detail != "focused" ||
        payload.detail1 != (index == 0 ? 1 : 0) || payload.detail2 != 0 ||
        !payload.value.empty() ||
        payload.source != test::AtspiObjectReference{
            ":1.88", "/org/a11y/atspi/accessible/10"}) {
      return 3;
    }
  }

  const auto diagnostics = wayland_atspi_event_diagnostics(*adapter);
  if (diagnostics.focus_event_count != 2 ||
      diagnostics.send_failure_count != 0) {
    return 4;
  }

  wayland_atspi_detach_dbus(*adapter);
  return 0;
}
