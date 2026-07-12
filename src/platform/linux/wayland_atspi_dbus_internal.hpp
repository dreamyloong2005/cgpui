#pragma once

#include "wayland_atspi_object_internal.hpp"

#include <dbus/dbus.h>

#include <cstddef>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace cgpui {

struct WaylandAtspiDbusOperations {
  dbus_bool_t (*register_object_path)(
      DBusConnection*,
      const char*,
      const DBusObjectPathVTable*,
      void*) = nullptr;
  dbus_bool_t (*unregister_object_path)(DBusConnection*, const char*) = nullptr;
  dbus_bool_t (*send)(DBusConnection*, DBusMessage*, dbus_uint32_t*) = nullptr;
  const char* (*get_unique_name)(DBusConnection*) = nullptr;
};

struct WaylandAtspiDbusDiagnostics {
  std::size_t registration_attempt_count = 0;
  std::size_t failed_registration_count = 0;
  std::size_t registered_object_count = 0;
  std::size_t unregistration_count = 0;
  std::size_t method_call_count = 0;
  std::size_t unknown_object_count = 0;
};

[[nodiscard]] WaylandAtspiDbusOperations
default_wayland_atspi_dbus_operations();

class WaylandAtspiDbusRegistry {
 public:
  ~WaylandAtspiDbusRegistry();

  void attach(
      DBusConnection* connection,
      WaylandAtspiDbusOperations operations =
          default_wayland_atspi_dbus_operations());
  void detach();
  void set_root_object_path(std::string root_object_path);
  void synchronize(std::span<const WaylandAtspiObjectNode> objects);
  [[nodiscard]] WaylandAtspiDbusDiagnostics diagnostics() const;
  static DBusHandlerResult handle_message(
      DBusConnection* connection,
      DBusMessage* message,
      void* user_data);

 private:
  DBusHandlerResult dispatch(
      DBusConnection* connection,
      DBusMessage* message);
  bool send_reply(DBusConnection* connection, DBusMessage* reply) const;

  DBusConnection* connection_ = nullptr;
  WaylandAtspiDbusOperations operations_;
  std::unordered_map<std::string, WaylandAtspiObjectNode> objects_;
  std::vector<std::string> registered_paths_;
  std::string root_object_path_;
  WaylandAtspiDbusDiagnostics diagnostics_;
};

} // namespace cgpui
