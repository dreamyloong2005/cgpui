#include "wayland_accessibility_internal.hpp"
#include "wayland_atspi_bus_internal.hpp"
#include "wayland_atspi_dbus_test_support.hpp"
#include "wayland_services_internal.hpp"

#include <dbus/dbus.h>

#include <string>

namespace {

class BusRecorder {
 public:
  explicit BusRecorder(DBusConnection* accessibility_connection)
      : accessibility_connection_(accessibility_connection) {}

  cgpui::WaylandAtspiBusOperations operations() {
    active_ = this;
    return cgpui::WaylandAtspiBusOperations{
        .get_session_bus = &get_session_bus,
        .send_with_reply_and_block = &send_with_reply_and_block,
        .open_private = &open_private,
        .register_connection = &register_connection,
        .set_exit_on_disconnect = &set_exit_on_disconnect,
        .close = &close,
        .unref = &unref,
    };
  }

  bool return_address = true;
  bool open_succeeds = true;
  bool register_succeeds = true;
  std::size_t discovery_calls = 0;
  std::size_t open_calls = 0;
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
    if (reply == nullptr ||
        !dbus_message_append_args(
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
    return active_->register_succeeds;
  }

  static void set_exit_on_disconnect(DBusConnection*, dbus_bool_t value) {
    active_->disabled_exit_on_disconnect = value == false;
  }

  static void close(DBusConnection*) { active_->close_calls += 1; }

  static void unref(DBusConnection* connection) {
    if (connection == active_->session_connection_) {
      active_->session_unref_calls += 1;
    } else if (connection == active_->accessibility_connection_) {
      active_->accessibility_unref_calls += 1;
    }
  }

  inline static BusRecorder* active_ = nullptr;
  DBusConnection* session_connection_ =
      reinterpret_cast<DBusConnection*>(0xCA11);
  DBusConnection* accessibility_connection_ = nullptr;
};

} // namespace

int main() {
  using namespace cgpui;

  test::WaylandAtspiDbusRecorder dbus_recorder;
  BusRecorder bus_recorder(dbus_recorder.connection());
  auto adapter = create_wayland_atspi_accessibility_adapter();
  if (!adapter->ensure_dbus_connection(
          bus_recorder.operations(), dbus_recorder.operations())) {
    return 1;
  }

  PlatformAccessibilityTreeUpdate update;
  update.root_element_id = 10;
  update.node_count = 1;
  update.nodes.push_back(PlatformAccessibilityNodeUpdate{
      .element_id = 10,
      .role = PlatformAccessibilityRole::button,
      .name = "Save",
  });
  wayland_atspi_update_accessibility_tree(*adapter, std::move(update));

  const auto diagnostics = adapter->bus_diagnostics();
  if (bus_recorder.discovery_calls != 1 || bus_recorder.open_calls != 1 ||
      bus_recorder.opened_address != "unix:path=/tmp/cgpui-atspi" ||
      !bus_recorder.disabled_exit_on_disconnect ||
      diagnostics.discovery_attempt_count != 1 ||
      diagnostics.connection_attempt_count != 1 || !diagnostics.connected ||
      dbus_recorder.registered_paths.size() != 1) {
    return 2;
  }

  wayland_atspi_detach_dbus(*adapter);
  if (bus_recorder.close_calls != 1 ||
      bus_recorder.session_unref_calls != 1 ||
      bus_recorder.accessibility_unref_calls != 1 ||
      adapter->bus_diagnostics().disconnect_count != 1) {
    return 3;
  }

  BusRecorder failed_discovery(dbus_recorder.connection());
  failed_discovery.return_address = false;
  WaylandAtspiBusConnection discovery_failure;
  if (discovery_failure.connect(failed_discovery.operations()) ||
      discovery_failure.diagnostics().discovery_failure_count != 1 ||
      failed_discovery.open_calls != 0) {
    return 4;
  }

  BusRecorder failed_open(dbus_recorder.connection());
  failed_open.open_succeeds = false;
  WaylandAtspiBusConnection connection_failure;
  if (connection_failure.connect(failed_open.operations()) ||
      connection_failure.diagnostics().connection_failure_count != 1 ||
      failed_open.open_calls != 1) {
    return 5;
  }

  BusRecorder failed_register(dbus_recorder.connection());
  failed_register.register_succeeds = false;
  WaylandAtspiBusConnection registration_failure;
  if (registration_failure.connect(failed_register.operations()) ||
      registration_failure.diagnostics().connection_failure_count != 1 ||
      failed_register.close_calls != 1 ||
      failed_register.accessibility_unref_calls != 1) {
    return 6;
  }
  return 0;
}
