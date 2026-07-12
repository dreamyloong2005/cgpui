#pragma once

#include "wayland_atspi_bus_internal.hpp"

#include <dbus/dbus.h>

#include <cstddef>
#include <string>

namespace cgpui::test {

class WaylandAtspiBusRecorder {
 public:
  explicit WaylandAtspiBusRecorder(DBusConnection* accessibility_connection)
      : accessibility_connection_(accessibility_connection) {}

  WaylandAtspiBusOperations operations() {
    active_ = this;
    return WaylandAtspiBusOperations{
        .get_session_bus = &get_session_bus,
        .send_with_reply_and_block = &send_with_reply_and_block,
        .open_private = &open_private,
        .register_connection = &register_connection,
        .set_exit_on_disconnect = &set_exit_on_disconnect,
        .get_is_connected = &get_is_connected,
        .close = &close,
        .unref = &unref,
    };
  }

  void report_disconnect_once() { disconnected_checks_remaining = 1; }

  bool return_address = true;
  bool open_succeeds = true;
  bool register_succeeds = true;
  std::size_t disconnected_checks_remaining = 0;
  std::size_t discovery_calls = 0;
  std::size_t open_calls = 0;
  std::size_t register_calls = 0;
  std::size_t connected_checks = 0;
  std::size_t close_calls = 0;
  std::size_t session_unref_calls = 0;
  std::size_t accessibility_unref_calls = 0;
  bool disabled_exit_on_disconnect = false;
  std::string opened_address;

 private:
  static DBusConnection* get_session_bus(DBusError*) {
    active_->discovery_calls += 1;
    return active_->session_connection_;
  }

  static DBusMessage* send_with_reply_and_block(
      DBusConnection*, DBusMessage* message, int, DBusError*) {
    if (std::string(dbus_message_get_destination(message)) != "org.a11y.Bus" ||
        std::string(dbus_message_get_path(message)) != "/org/a11y/bus" ||
        std::string(dbus_message_get_interface(message)) != "org.a11y.Bus" ||
        std::string(dbus_message_get_member(message)) != "GetAddress") {
      return nullptr;
    }
    if (!active_->return_address) return nullptr;
    DBusMessage* reply = dbus_message_new(DBUS_MESSAGE_TYPE_METHOD_RETURN);
    const char* address = "unix:path=/tmp/cgpui-atspi";
    if (reply == nullptr || !dbus_message_append_args(
            reply, DBUS_TYPE_STRING, &address, DBUS_TYPE_INVALID)) {
      if (reply != nullptr) dbus_message_unref(reply);
      return nullptr;
    }
    return reply;
  }

  static DBusConnection* open_private(const char* address, DBusError*) {
    active_->open_calls += 1;
    active_->opened_address = address == nullptr ? "" : address;
    return active_->open_succeeds ? active_->accessibility_connection_ : nullptr;
  }

  static dbus_bool_t register_connection(DBusConnection*, DBusError*) {
    active_->register_calls += 1;
    return active_->register_succeeds;
  }

  static void set_exit_on_disconnect(DBusConnection*, dbus_bool_t value) {
    active_->disabled_exit_on_disconnect = value == false;
  }

  static dbus_bool_t get_is_connected(DBusConnection*) {
    active_->connected_checks += 1;
    if (active_->disconnected_checks_remaining == 0) return true;
    active_->disconnected_checks_remaining -= 1;
    return false;
  }

  static void close(DBusConnection*) { active_->close_calls += 1; }

  static void unref(DBusConnection* connection) {
    if (connection == active_->session_connection_) {
      active_->session_unref_calls += 1;
    } else if (connection == active_->accessibility_connection_) {
      active_->accessibility_unref_calls += 1;
    }
  }

  inline static WaylandAtspiBusRecorder* active_ = nullptr;
  DBusConnection* session_connection_ =
      reinterpret_cast<DBusConnection*>(0xCA11);
  DBusConnection* accessibility_connection_ = nullptr;
};

} // namespace cgpui::test
