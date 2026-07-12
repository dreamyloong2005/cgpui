#include "wayland_atspi_dbus_internal.hpp"
#include "wayland_atspi_dbus_messages_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

dbus_bool_t register_object_path(
    DBusConnection* connection,
    const char* path,
    const DBusObjectPathVTable* vtable,
    void* user_data) {
  return dbus_connection_register_object_path(
      connection, path, vtable, user_data);
}

dbus_bool_t unregister_object_path(
    DBusConnection* connection, const char* path) {
  return dbus_connection_unregister_object_path(connection, path);
}

dbus_bool_t send_message(
    DBusConnection* connection,
    DBusMessage* message,
    dbus_uint32_t* serial) {
  return dbus_connection_send(connection, message, serial);
}

const DBusObjectPathVTable& object_vtable() {
  static const DBusObjectPathVTable value{
      .unregister_function = nullptr,
      .message_function = &WaylandAtspiDbusRegistry::handle_message,
  };
  return value;
}

} // namespace

WaylandAtspiDbusOperations default_wayland_atspi_dbus_operations() {
  return WaylandAtspiDbusOperations{
      .register_object_path = &register_object_path,
      .unregister_object_path = &unregister_object_path,
      .send = &send_message,
  };
}

WaylandAtspiDbusRegistry::~WaylandAtspiDbusRegistry() {
  detach();
}

void WaylandAtspiDbusRegistry::attach(
    DBusConnection* connection, WaylandAtspiDbusOperations operations) {
  detach();
  connection_ = connection;
  operations_ = operations;
}

void WaylandAtspiDbusRegistry::detach() {
  if (connection_ != nullptr && operations_.unregister_object_path != nullptr) {
    for (auto path = registered_paths_.rbegin();
         path != registered_paths_.rend(); ++path) {
      if (operations_.unregister_object_path(connection_, path->c_str())) {
        diagnostics_.unregistration_count += 1;
      }
    }
  }
  registered_paths_.clear();
  objects_.clear();
  diagnostics_.registered_object_count = 0;
  connection_ = nullptr;
}

void WaylandAtspiDbusRegistry::synchronize(
    std::span<const WaylandAtspiObjectNode> objects) {
  std::unordered_map<std::string, WaylandAtspiObjectNode> replacements;
  replacements.reserve(objects.size());
  for (const WaylandAtspiObjectNode& object : objects) {
    replacements.insert_or_assign(object.object_path, object);
  }

  if (connection_ != nullptr && operations_.unregister_object_path != nullptr) {
    for (auto path = registered_paths_.begin();
         path != registered_paths_.end();) {
      if (replacements.contains(*path)) {
        ++path;
        continue;
      }
      if (operations_.unregister_object_path(connection_, path->c_str())) {
        diagnostics_.unregistration_count += 1;
      }
      path = registered_paths_.erase(path);
    }
  }

  objects_ = std::move(replacements);
  if (connection_ != nullptr && operations_.register_object_path != nullptr) {
    for (const WaylandAtspiObjectNode& object : objects) {
      if (std::ranges::find(registered_paths_, object.object_path) !=
          registered_paths_.end()) {
        continue;
      }
      diagnostics_.registration_attempt_count += 1;
      if (!operations_.register_object_path(
              connection_, object.object_path.c_str(), &object_vtable(), this)) {
        diagnostics_.failed_registration_count += 1;
        continue;
      }
      registered_paths_.push_back(object.object_path);
    }
  }
  diagnostics_.registered_object_count = registered_paths_.size();
}

WaylandAtspiDbusDiagnostics WaylandAtspiDbusRegistry::diagnostics() const {
  return diagnostics_;
}

DBusHandlerResult WaylandAtspiDbusRegistry::handle_message(
    DBusConnection* connection, DBusMessage* message, void* user_data) {
  if (user_data == nullptr) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  return static_cast<WaylandAtspiDbusRegistry*>(user_data)->dispatch(
      connection, message);
}

DBusHandlerResult WaylandAtspiDbusRegistry::dispatch(
    DBusConnection* connection, DBusMessage* message) {
  const char* path = dbus_message_get_path(message);
  if (path == nullptr || !objects_.contains(path)) {
    diagnostics_.unknown_object_count += 1;
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  WaylandAtspiDbusReply reply = wayland_atspi_dbus_reply_for(message);
  if (!reply.handled) {
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }
  diagnostics_.method_call_count += 1;
  if (reply.message == nullptr) return DBUS_HANDLER_RESULT_NEED_MEMORY;
  const bool sent = send_reply(connection, reply.message);
  dbus_message_unref(reply.message);
  return sent ? DBUS_HANDLER_RESULT_HANDLED
              : DBUS_HANDLER_RESULT_NEED_MEMORY;
}

bool WaylandAtspiDbusRegistry::send_reply(
    DBusConnection* connection, DBusMessage* reply) const {
  return operations_.send != nullptr &&
         operations_.send(connection, reply, nullptr);
}

} // namespace cgpui
