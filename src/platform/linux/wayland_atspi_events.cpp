#include "wayland_atspi_events_internal.hpp"

#include <algorithm>
#include <cstdint>
#include <string_view>

namespace cgpui {
namespace {

constexpr const char* event_interface = "org.a11y.atspi.Event.Object";

std::size_t utf8_code_point_count(std::string_view text) {
  return static_cast<std::size_t>(std::ranges::count_if(
      text, [](unsigned char value) { return (value & 0xC0U) != 0x80U; }));
}

bool append_source(
    DBusMessageIter& root,
    const char* bus_name,
    const char* object_path) {
  DBusMessageIter source;
  return dbus_message_iter_open_container(
             &root, DBUS_TYPE_STRUCT, nullptr, &source) &&
         dbus_message_iter_append_basic(
             &source, DBUS_TYPE_STRING, &bus_name) &&
         dbus_message_iter_append_basic(
             &source, DBUS_TYPE_OBJECT_PATH, &object_path) &&
         dbus_message_iter_close_container(&root, &source);
}

DBusMessage* event_message(
    const WaylandAtspiObjectNode& object,
    const char* member,
    const char* detail,
    std::int32_t detail1,
    std::int32_t detail2,
    const std::string& value,
    const char* bus_name) {
  DBusMessage* message = dbus_message_new_signal(
      object.object_path.c_str(), event_interface, member);
  if (message == nullptr) return nullptr;
  DBusMessageIter root;
  DBusMessageIter variant;
  dbus_message_iter_init_append(message, &root);
  const char* payload = value.c_str();
  const char* source_path = object.object_path.c_str();
  if (!dbus_message_iter_append_basic(
          &root, DBUS_TYPE_STRING, &detail) ||
      !dbus_message_iter_append_basic(
          &root, DBUS_TYPE_INT32, &detail1) ||
      !dbus_message_iter_append_basic(
          &root, DBUS_TYPE_INT32, &detail2) ||
      !dbus_message_iter_open_container(
          &root, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant) ||
      !dbus_message_iter_append_basic(
          &variant, DBUS_TYPE_STRING, &payload) ||
      !dbus_message_iter_close_container(&root, &variant) ||
      !append_source(root, bus_name, source_path)) {
    dbus_message_unref(message);
    return nullptr;
  }
  return message;
}

} // namespace

void WaylandAtspiEventPublisher::attach(
    DBusConnection* connection, WaylandAtspiDbusOperations operations) {
  connection_ = connection;
  operations_ = operations;
}

void WaylandAtspiEventPublisher::detach() {
  connection_ = nullptr;
  operations_ = {};
}

void WaylandAtspiEventPublisher::publish(
    std::span<const WaylandAtspiObjectNode> objects,
    std::span<const PlatformAccessibilityLiveUpdate> updates) {
  for (const PlatformAccessibilityLiveUpdate& update : updates) {
    const auto object = std::ranges::find(
        objects, update.element_id, &WaylandAtspiObjectNode::element_id);
    if (object == objects.end()) {
      diagnostics_.missing_object_count += 1;
      continue;
    }
    if (connection_ == nullptr || operations_.send == nullptr) {
      diagnostics_.no_connection_count += 1;
      continue;
    }
    const char* unique_name = operations_.get_unique_name == nullptr
        ? ""
        : operations_.get_unique_name(connection_);
    DBusMessage* message = nullptr;
    if (update.kind == PlatformAccessibilityLiveUpdateKind::value_changed) {
      message = event_message(
          *object, "PropertyChange", "accessible-value", 0, 0,
          update.value, unique_name == nullptr ? "" : unique_name);
    } else if (update.kind ==
               PlatformAccessibilityLiveUpdateKind::text_changed) {
      message = event_message(
          *object, "TextChanged", "insert", 0,
          static_cast<std::int32_t>(utf8_code_point_count(update.text)),
          update.text, unique_name == nullptr ? "" : unique_name);
    } else if (update.kind ==
               PlatformAccessibilityLiveUpdateKind::focus_changed) {
      message = event_message(
          *object, "StateChanged", "focused", update.focused ? 1 : 0, 0,
          {}, unique_name == nullptr ? "" : unique_name);
    }
    if (message == nullptr ||
        !operations_.send(connection_, message, nullptr)) {
      diagnostics_.send_failure_count += 1;
      if (message != nullptr) dbus_message_unref(message);
      continue;
    }
    if (update.kind == PlatformAccessibilityLiveUpdateKind::value_changed) {
      diagnostics_.value_event_count += 1;
    } else if (update.kind ==
               PlatformAccessibilityLiveUpdateKind::text_changed) {
      diagnostics_.text_event_count += 1;
    } else {
      diagnostics_.focus_event_count += 1;
    }
    dbus_message_unref(message);
  }
}

WaylandAtspiEventDiagnostics WaylandAtspiEventPublisher::diagnostics() const {
  return diagnostics_;
}

} // namespace cgpui
