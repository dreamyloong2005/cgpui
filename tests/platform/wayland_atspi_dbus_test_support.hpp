#pragma once

#include "wayland_atspi_dbus_internal.hpp"

#include <dbus/dbus.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace cgpui::test {

struct AtspiObjectReference {
  std::string bus_name;
  std::string object_path;

  bool operator==(const AtspiObjectReference&) const = default;
};

class WaylandAtspiDbusRecorder {
 public:
  struct RegisteredPath {
    std::string path;
    const DBusObjectPathVTable* vtable = nullptr;
    void* user_data = nullptr;
  };

  ~WaylandAtspiDbusRecorder() {
    clear_reply();
    clear_messages();
  }

  WaylandAtspiDbusOperations operations() {
    active_ = this;
    return WaylandAtspiDbusOperations{
        .register_object_path = &register_object_path,
        .unregister_object_path = &unregister_object_path,
        .send = &send_message,
        .get_unique_name = &get_unique_name,
    };
  }

  [[nodiscard]] DBusConnection* connection() const {
    return reinterpret_cast<DBusConnection*>(static_cast<std::uintptr_t>(1));
  }

  DBusHandlerResult call(DBusMessage* message, std::size_t registration) {
    clear_reply();
    if (message == nullptr || registration >= registered_paths.size()) {
      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    const RegisteredPath& path = registered_paths[registration];
    return path.vtable->message_function(connection(), message, path.user_data);
  }

  void clear_reply() {
    if (sent_reply != nullptr) {
      dbus_message_unref(sent_reply);
      sent_reply = nullptr;
    }
  }

  void clear_messages() {
    for (DBusMessage* message : sent_messages) dbus_message_unref(message);
    sent_messages.clear();
  }

  std::string unique_name = ":1.88";
  bool send_result = true;
  std::vector<RegisteredPath> registered_paths;
  std::vector<std::string> unregistered_paths;
  std::vector<DBusMessage*> sent_messages;
  DBusMessage* sent_reply = nullptr;
 private:
  static dbus_bool_t register_object_path(
      DBusConnection*,
      const char* path,
      const DBusObjectPathVTable* vtable,
      void* user_data) {
    active_->registered_paths.push_back({path, vtable, user_data});
    return true;
  }

  static dbus_bool_t unregister_object_path(DBusConnection*, const char* path) {
    active_->unregistered_paths.emplace_back(path);
    return true;
  }

  static dbus_bool_t send_message(
      DBusConnection*, DBusMessage* message, dbus_uint32_t*) {
    if (active_->sent_reply != nullptr) {
      dbus_message_unref(active_->sent_reply);
    }
    active_->sent_reply = dbus_message_ref(message);
    active_->sent_messages.push_back(dbus_message_ref(message));
    return active_->send_result;
  }

  static const char* get_unique_name(DBusConnection*) {
    return active_->unique_name.c_str();
  }

  static inline WaylandAtspiDbusRecorder* active_ = nullptr;
};

inline DBusMessage* atspi_method_call(
    const char* path, const char* interface_name, const char* member) {
  static dbus_uint32_t serial = 100;
  DBusMessage* message = dbus_message_new_method_call(
      nullptr, path, interface_name, member);
  if (message != nullptr) dbus_message_set_serial(message, serial++);
  return message;
}

inline DBusMessage* atspi_indexed_method_call(
    const char* path, const char* member, std::int32_t index) {
  DBusMessage* message = atspi_method_call(
      path, "org.a11y.atspi.Accessible", member);
  if (message == nullptr || !dbus_message_append_args(
          message, DBUS_TYPE_INT32, &index, DBUS_TYPE_INVALID)) {
    if (message != nullptr) dbus_message_unref(message);
    return nullptr;
  }
  return message;
}

inline DBusMessage* atspi_property_get(
    const char* path, const char* property_name) {
  DBusMessage* message = atspi_method_call(
      path, DBUS_INTERFACE_PROPERTIES, "Get");
  const char* interface_name = "org.a11y.atspi.Accessible";
  if (message == nullptr || !dbus_message_append_args(
          message,
          DBUS_TYPE_STRING, &interface_name,
          DBUS_TYPE_STRING, &property_name,
          DBUS_TYPE_INVALID)) {
    if (message != nullptr) dbus_message_unref(message);
    return nullptr;
  }
  return message;
}

inline AtspiObjectReference read_object_reference(DBusMessageIter* iterator) {
  DBusMessageIter structure;
  dbus_message_iter_recurse(iterator, &structure);
  const char* bus_name = nullptr;
  const char* object_path = nullptr;
  dbus_message_iter_get_basic(&structure, &bus_name);
  dbus_message_iter_next(&structure);
  dbus_message_iter_get_basic(&structure, &object_path);
  return {
      bus_name == nullptr ? "" : bus_name,
      object_path == nullptr ? "" : object_path,
  };
}

inline AtspiObjectReference reply_object_reference(
    DBusMessage* reply, bool property_variant = false) {
  DBusMessageIter root;
  if (reply == nullptr || !dbus_message_iter_init(reply, &root)) return {};
  if (property_variant) {
    DBusMessageIter variant;
    dbus_message_iter_recurse(&root, &variant);
    return read_object_reference(&variant);
  }
  return read_object_reference(&root);
}

inline std::int32_t reply_int32(
    DBusMessage* reply, bool property_variant = false) {
  DBusMessageIter root;
  if (reply == nullptr || !dbus_message_iter_init(reply, &root)) return -1;
  if (property_variant) {
    DBusMessageIter variant;
    dbus_message_iter_recurse(&root, &variant);
    std::int32_t value = -1;
    dbus_message_iter_get_basic(&variant, &value);
    return value;
  }
  std::int32_t value = -1;
  dbus_message_iter_get_basic(&root, &value);
  return value;
}

inline std::vector<AtspiObjectReference> reply_object_references(
    DBusMessage* reply) {
  DBusMessageIter root;
  if (reply == nullptr || !dbus_message_iter_init(reply, &root)) return {};
  DBusMessageIter array;
  dbus_message_iter_recurse(&root, &array);
  std::vector<AtspiObjectReference> references;
  while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_STRUCT) {
    references.push_back(read_object_reference(&array));
    dbus_message_iter_next(&array);
  }
  return references;
}

inline std::string reply_string(DBusMessage* reply) {
  DBusError error;
  dbus_error_init(&error);
  const char* value = nullptr;
  if (reply == nullptr || !dbus_message_get_args(
          reply, &error, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
    dbus_error_free(&error);
    return {};
  }
  return value == nullptr ? std::string{} : std::string(value);
}

inline std::vector<std::string> reply_string_array(DBusMessage* reply) {
  DBusMessageIter root;
  if (reply == nullptr || !dbus_message_iter_init(reply, &root) ||
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

} // namespace cgpui::test
