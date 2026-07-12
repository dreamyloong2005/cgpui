#pragma once

#include <dbus/dbus.h>

#include <cstddef>

namespace cgpui {

struct WaylandAtspiBusOperations {
  DBusConnection* (*get_session_bus)(DBusError*) = nullptr;
  DBusMessage* (*send_with_reply_and_block)(
      DBusConnection*, DBusMessage*, int, DBusError*) = nullptr;
  DBusConnection* (*open_private)(const char*, DBusError*) = nullptr;
  dbus_bool_t (*register_connection)(DBusConnection*, DBusError*) = nullptr;
  void (*set_exit_on_disconnect)(DBusConnection*, dbus_bool_t) = nullptr;
  dbus_bool_t (*get_is_connected)(DBusConnection*) = nullptr;
  void (*close)(DBusConnection*) = nullptr;
  void (*unref)(DBusConnection*) = nullptr;
};

struct WaylandAtspiBusDiagnostics {
  std::size_t discovery_attempt_count = 0;
  std::size_t discovery_failure_count = 0;
  std::size_t connection_attempt_count = 0;
  std::size_t connection_failure_count = 0;
  std::size_t connection_loss_count = 0;
  std::size_t reconnect_attempt_count = 0;
  std::size_t reconnect_success_count = 0;
  std::size_t reconnect_failure_count = 0;
  std::size_t disconnect_count = 0;
  bool connected = false;
};

[[nodiscard]] WaylandAtspiBusOperations
default_wayland_atspi_bus_operations();

class WaylandAtspiBusConnection {
 public:
  WaylandAtspiBusConnection() = default;
  WaylandAtspiBusConnection(const WaylandAtspiBusConnection&) = delete;
  WaylandAtspiBusConnection& operator=(const WaylandAtspiBusConnection&) = delete;
  ~WaylandAtspiBusConnection();

  bool connect(
      WaylandAtspiBusOperations operations =
          default_wayland_atspi_bus_operations());
  [[nodiscard]] bool is_connected();
  void disconnect();
  [[nodiscard]] DBusConnection* connection() const;
  [[nodiscard]] WaylandAtspiBusDiagnostics diagnostics() const;

 private:
  DBusConnection* connection_ = nullptr;
  WaylandAtspiBusOperations operations_;
  WaylandAtspiBusDiagnostics diagnostics_;
  bool reconnect_pending_ = false;
};

} // namespace cgpui
