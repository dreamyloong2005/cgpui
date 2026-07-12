#include "wayland_accessibility_internal.hpp"
#include "wayland_services_internal.hpp"

#include <dbus/dbus.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace {

struct RegisteredPath {
  std::string path;
  const DBusObjectPathVTable* vtable = nullptr;
  void* user_data = nullptr;
};

std::vector<RegisteredPath> registered_paths;
std::vector<std::string> unregistered_paths;
DBusMessage* sent_reply = nullptr;

dbus_bool_t register_object_path(
    DBusConnection*,
    const char* path,
    const DBusObjectPathVTable* vtable,
    void* user_data) {
  registered_paths.push_back(RegisteredPath{
      .path = path,
      .vtable = vtable,
      .user_data = user_data,
  });
  return true;
}

dbus_bool_t unregister_object_path(DBusConnection*, const char* path) {
  unregistered_paths.emplace_back(path);
  return true;
}

dbus_bool_t send_message(
    DBusConnection*, DBusMessage* message, dbus_uint32_t*) {
  if (sent_reply != nullptr) {
    dbus_message_unref(sent_reply);
  }
  sent_reply = dbus_message_ref(message);
  return true;
}

void clear_reply() {
  if (sent_reply != nullptr) {
    dbus_message_unref(sent_reply);
    sent_reply = nullptr;
  }
}

DBusMessage* method_call(
    const char* path, const char* interface_name, const char* member) {
  static dbus_uint32_t serial = 1;
  DBusMessage* message = dbus_message_new_method_call(
      nullptr, path, interface_name, member);
  if (message != nullptr) {
    dbus_message_set_serial(message, serial++);
  }
  return message;
}

std::string reply_string(DBusMessage* reply) {
  DBusError error;
  dbus_error_init(&error);
  const char* value = nullptr;
  if (!dbus_message_get_args(
          reply, &error, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
    dbus_error_free(&error);
    return {};
  }
  return value == nullptr ? std::string{} : std::string(value);
}

std::vector<std::string> reply_string_array(DBusMessage* reply) {
  DBusMessageIter root;
  if (!dbus_message_iter_init(reply, &root) ||
      dbus_message_iter_get_arg_type(&root) != DBUS_TYPE_ARRAY) {
    return {};
  }
  DBusMessageIter array;
  dbus_message_iter_recurse(&root, &array);
  std::vector<std::string> values;
  while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_STRING) {
    const char* value = nullptr;
    dbus_message_iter_get_basic(&array, &value);
    values.emplace_back(value == nullptr ? "" : value);
    dbus_message_iter_next(&array);
  }
  return values;
}

} // namespace

int main() {
  using namespace cgpui;

  auto adapter = create_wayland_atspi_accessibility_adapter();
  const WaylandAtspiDbusOperations operations{
      .register_object_path = &register_object_path,
      .unregister_object_path = &unregister_object_path,
      .send = &send_message,
  };
  auto* connection = reinterpret_cast<DBusConnection*>(
      static_cast<std::uintptr_t>(1));
  wayland_atspi_attach_dbus(*adapter, connection, operations);

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

  if (registered_paths.size() != 2 ||
      registered_paths[0].path != "/org/a11y/atspi/accessible/10" ||
      registered_paths[1].path != "/org/a11y/atspi/accessible/11" ||
      registered_paths[1].vtable == nullptr ||
      registered_paths[1].vtable->message_function == nullptr) {
    return 1;
  }

  DBusMessage* introspect = method_call(
      registered_paths[1].path.c_str(),
      DBUS_INTERFACE_INTROSPECTABLE,
      "Introspect");
  if (introspect == nullptr ||
      registered_paths[1].vtable->message_function(
          connection, introspect, registered_paths[1].user_data) !=
          DBUS_HANDLER_RESULT_HANDLED ||
      sent_reply == nullptr ||
      dbus_message_get_type(sent_reply) != DBUS_MESSAGE_TYPE_METHOD_RETURN) {
    if (introspect != nullptr) dbus_message_unref(introspect);
    clear_reply();
    return 2;
  }
  const std::string xml = reply_string(sent_reply);
  dbus_message_unref(introspect);
  clear_reply();
  if (xml.find("org.a11y.atspi.Accessible") == std::string::npos ||
      xml.find("org.freedesktop.DBus.Introspectable") == std::string::npos ||
      xml.find("GetInterfaces") == std::string::npos) {
    return 3;
  }

  DBusMessage* interfaces = method_call(
      registered_paths[1].path.c_str(),
      "org.a11y.atspi.Accessible",
      "GetInterfaces");
  if (interfaces == nullptr ||
      registered_paths[1].vtable->message_function(
          connection, interfaces, registered_paths[1].user_data) !=
          DBUS_HANDLER_RESULT_HANDLED ||
      sent_reply == nullptr) {
    if (interfaces != nullptr) dbus_message_unref(interfaces);
    clear_reply();
    return 4;
  }
  const auto exposed_interfaces = reply_string_array(sent_reply);
  dbus_message_unref(interfaces);
  clear_reply();
  if (exposed_interfaces != std::vector<std::string>{
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
  if (unregistered_paths !=
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
  if (unregistered_paths != std::vector<std::string>{
          "/org/a11y/atspi/accessible/11",
          "/org/a11y/atspi/accessible/10"}) {
    return 8;
  }
  clear_reply();
  return 0;
}
