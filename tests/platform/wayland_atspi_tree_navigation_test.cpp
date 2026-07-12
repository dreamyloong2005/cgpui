#include "wayland_accessibility_internal.hpp"
#include "wayland_atspi_dbus_test_support.hpp"
#include "wayland_services_internal.hpp"

#include <dbus/dbus.h>

#include <cstdint>
#include <string>
#include <vector>

namespace {

using cgpui::test::AtspiObjectReference;
using cgpui::test::WaylandAtspiDbusRecorder;

AtspiObjectReference call_object(
    WaylandAtspiDbusRecorder& recorder,
    std::size_t registration,
    DBusMessage* request,
    bool property = false) {
  if (recorder.call(request, registration) != DBUS_HANDLER_RESULT_HANDLED) {
    if (request != nullptr) dbus_message_unref(request);
    return {};
  }
  dbus_message_unref(request);
  return cgpui::test::reply_object_reference(recorder.sent_reply, property);
}

std::int32_t call_int32(
    WaylandAtspiDbusRecorder& recorder,
    std::size_t registration,
    DBusMessage* request,
    bool property = false) {
  if (recorder.call(request, registration) != DBUS_HANDLER_RESULT_HANDLED) {
    if (request != nullptr) dbus_message_unref(request);
    return -1;
  }
  dbus_message_unref(request);
  return cgpui::test::reply_int32(recorder.sent_reply, property);
}

} // namespace

int main() {
  using namespace cgpui;

  WaylandAtspiDbusRecorder recorder;
  auto adapter = create_wayland_atspi_accessibility_adapter();
  wayland_atspi_attach_dbus(
      *adapter, recorder.connection(), recorder.operations());

  PlatformAccessibilityTreeUpdate update;
  update.root_element_id = 10;
  update.node_count = 4;
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = PlatformAccessibilityRole::generic,
      .name = "window",
      .child_count = 2,
  });
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 11,
      .parent_element_id = 10,
      .role = PlatformAccessibilityRole::generic,
      .name = "group",
      .child_count = 1,
  });
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 13,
      .parent_element_id = 11,
      .role = PlatformAccessibilityRole::button,
      .name = "nested",
  });
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 12,
      .parent_element_id = 10,
      .role = PlatformAccessibilityRole::button,
      .name = "sibling",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(update));
  if (recorder.registered_paths.size() != 4) return 1;

  const AtspiObjectReference parent = call_object(
      recorder, 2,
      test::atspi_property_get(
          "/org/a11y/atspi/accessible/13", "Parent"),
      true);
  if (parent != AtspiObjectReference{
          ":1.88", "/org/a11y/atspi/accessible/11"}) {
    return 2;
  }

  const std::int32_t child_count = call_int32(
      recorder, 0,
      test::atspi_property_get(
          "/org/a11y/atspi/accessible/10", "ChildCount"),
      true);
  if (child_count != 2) return 3;

  const AtspiObjectReference child = call_object(
      recorder, 0,
      test::atspi_indexed_method_call(
          "/org/a11y/atspi/accessible/10", "GetChildAtIndex", 1));
  if (child != AtspiObjectReference{
          ":1.88", "/org/a11y/atspi/accessible/12"}) {
    return 4;
  }

  DBusMessage* children_request = test::atspi_method_call(
      "/org/a11y/atspi/accessible/10",
      "org.a11y.atspi.Accessible",
      "GetChildren");
  if (recorder.call(children_request, 0) != DBUS_HANDLER_RESULT_HANDLED) {
    dbus_message_unref(children_request);
    return 5;
  }
  dbus_message_unref(children_request);
  if (test::reply_object_references(recorder.sent_reply) !=
      std::vector<AtspiObjectReference>{
          {":1.88", "/org/a11y/atspi/accessible/11"},
          {":1.88", "/org/a11y/atspi/accessible/12"}}) {
    return 6;
  }

  if (call_int32(
          recorder, 2,
          test::atspi_method_call(
              "/org/a11y/atspi/accessible/13",
              "org.a11y.atspi.Accessible",
              "GetIndexInParent")) != 0 ||
      call_int32(
          recorder, 3,
          test::atspi_method_call(
              "/org/a11y/atspi/accessible/12",
              "org.a11y.atspi.Accessible",
              "GetIndexInParent")) != 1) {
    return 7;
  }

  const AtspiObjectReference application = call_object(
      recorder, 2,
      test::atspi_method_call(
          "/org/a11y/atspi/accessible/13",
          "org.a11y.atspi.Accessible",
          "GetApplication"));
  if (application != AtspiObjectReference{
          ":1.88", "/org/a11y/atspi/accessible/10"}) {
    return 8;
  }

  wayland_atspi_detach_dbus(*adapter);
  return 0;
}
