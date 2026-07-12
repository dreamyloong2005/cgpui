#include "wayland_atspi_bus_internal.hpp"

#include <string>

namespace cgpui {
namespace {

DBusConnection* get_session_bus(DBusError* error) {
  return dbus_bus_get(DBUS_BUS_SESSION, error);
}

DBusMessage* send_with_reply_and_block(
    DBusConnection* connection,
    DBusMessage* message,
    int timeout,
    DBusError* error) {
  return dbus_connection_send_with_reply_and_block(
      connection, message, timeout, error);
}

DBusConnection* open_private(const char* address, DBusError* error) {
  return dbus_connection_open_private(address, error);
}

dbus_bool_t register_connection(
    DBusConnection* connection, DBusError* error) {
  return dbus_bus_register(connection, error);
}

void set_exit_on_disconnect(DBusConnection* connection, dbus_bool_t value) {
  dbus_connection_set_exit_on_disconnect(connection, value);
}

void close_connection(DBusConnection* connection) {
  dbus_connection_close(connection);
}

void unref_connection(DBusConnection* connection) {
  dbus_connection_unref(connection);
}

void free_error(DBusError& error) {
  if (dbus_error_is_set(&error)) dbus_error_free(&error);
}

} // namespace

WaylandAtspiBusOperations default_wayland_atspi_bus_operations() {
  return WaylandAtspiBusOperations{
      .get_session_bus = &get_session_bus,
      .send_with_reply_and_block = &send_with_reply_and_block,
      .open_private = &open_private,
      .register_connection = &register_connection,
      .set_exit_on_disconnect = &set_exit_on_disconnect,
      .close = &close_connection,
      .unref = &unref_connection,
  };
}

WaylandAtspiBusConnection::~WaylandAtspiBusConnection() {
  disconnect();
}

bool WaylandAtspiBusConnection::connect(
    WaylandAtspiBusOperations operations) {
  disconnect();
  operations_ = operations;
  diagnostics_.discovery_attempt_count += 1;

  DBusError error;
  dbus_error_init(&error);
  DBusConnection* session = operations_.get_session_bus == nullptr
      ? nullptr
      : operations_.get_session_bus(&error);
  DBusMessage* request = session == nullptr ? nullptr
      : dbus_message_new_method_call(
            "org.a11y.Bus", "/org/a11y/bus", "org.a11y.Bus", "GetAddress");
  DBusMessage* reply =
      request == nullptr || operations_.send_with_reply_and_block == nullptr
      ? nullptr
      : operations_.send_with_reply_and_block(
            session, request, DBUS_TIMEOUT_USE_DEFAULT, &error);
  if (request != nullptr) dbus_message_unref(request);

  const char* address_value = nullptr;
  const bool address_available =
      reply != nullptr &&
      dbus_message_get_args(
          reply, &error, DBUS_TYPE_STRING, &address_value, DBUS_TYPE_INVALID) &&
      address_value != nullptr && address_value[0] != '\0';
  const std::string address = address_available ? address_value : "";
  if (reply != nullptr) dbus_message_unref(reply);
  if (session != nullptr && operations_.unref != nullptr) {
    operations_.unref(session);
  }
  free_error(error);
  if (!address_available) {
    diagnostics_.discovery_failure_count += 1;
    return false;
  }

  diagnostics_.connection_attempt_count += 1;
  dbus_error_init(&error);
  DBusConnection* connection = operations_.open_private == nullptr
      ? nullptr
      : operations_.open_private(address.c_str(), &error);
  const bool registered =
      connection != nullptr && operations_.register_connection != nullptr &&
      operations_.register_connection(connection, &error);
  if (!registered) {
    if (connection != nullptr) {
      if (operations_.close != nullptr) operations_.close(connection);
      if (operations_.unref != nullptr) operations_.unref(connection);
    }
    free_error(error);
    diagnostics_.connection_failure_count += 1;
    return false;
  }
  free_error(error);
  if (operations_.set_exit_on_disconnect != nullptr) {
    operations_.set_exit_on_disconnect(connection, false);
  }
  connection_ = connection;
  diagnostics_.connected = true;
  return true;
}

void WaylandAtspiBusConnection::disconnect() {
  if (connection_ == nullptr) return;
  if (operations_.close != nullptr) operations_.close(connection_);
  if (operations_.unref != nullptr) operations_.unref(connection_);
  connection_ = nullptr;
  diagnostics_.connected = false;
  diagnostics_.disconnect_count += 1;
}

DBusConnection* WaylandAtspiBusConnection::connection() const {
  return connection_;
}

WaylandAtspiBusDiagnostics WaylandAtspiBusConnection::diagnostics() const {
  return diagnostics_;
}

} // namespace cgpui
