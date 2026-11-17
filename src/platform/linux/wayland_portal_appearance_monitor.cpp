#include "wayland_portal_appearance_monitor_internal.hpp"

#include <utility>

namespace cgpui {
namespace {

constexpr const char* match_rule =
    "type='signal',sender='org.freedesktop.portal.Desktop',"
    "interface='org.freedesktop.portal.Settings',member='SettingChanged',"
    "path='/org/freedesktop/portal/desktop'";

std::string error_text(const DBusError& error, const char* fallback) {
  if (!dbus_error_is_set(&error)) return fallback;
  std::string text = error.name == nullptr ? "" : error.name;
  if (error.message != nullptr) text += std::string(": ") + error.message;
  return text;
}

void free_error(DBusError& error) {
  if (dbus_error_is_set(&error)) dbus_error_free(&error);
}

} // namespace

WaylandPortalAppearanceMonitor::WaylandPortalAppearanceMonitor(
    std::string bus_address)
    : bus_address_(std::move(bus_address)) {
  mark_unavailable("xdg-desktop-portal not queried yet");
}

WaylandPortalAppearanceMonitor::~WaylandPortalAppearanceMonitor() {
  release_connection();
}

SystemAppearance WaylandPortalAppearanceMonitor::appearance() {
  ensure_connected();
  return appearance_;
}

void WaylandPortalAppearanceMonitor::set_callback(
    SystemAppearanceCallback callback) {
  callback_ = std::move(callback);
  if (callback_) ensure_connected();
}

const WaylandPortalAppearanceDiagnostics&
WaylandPortalAppearanceMonitor::diagnostics() const {
  return diagnostics_;
}

void WaylandPortalAppearanceMonitor::ensure_connected() {
  if (attempted_) return;
  attempted_ = true;
  diagnostics_.connection_attempt_count += 1;
  DBusError error;
  dbus_error_init(&error);
  if (!connect_bus(error)) {
    mark_unavailable(
        "session bus unavailable: " + error_text(error, "no connection"));
    free_error(error);
    release_connection();
    return;
  }
  // Subscribe before reading so a change between the two is not lost.
  if (!subscribe(error)) {
    mark_unavailable("SettingChanged subscription failed: " +
                     error_text(error, "out of memory"));
    free_error(error);
    release_connection();
    return;
  }
  if (!read_snapshot()) {
    mark_unavailable(
        "xdg-desktop-portal Settings unavailable: " + last_error_);
    release_connection();
    return;
  }
  diagnostics_.connected = true;
  diagnostics_.unavailable_reason.clear();
}

bool WaylandPortalAppearanceMonitor::connect_bus(DBusError& error) {
  if (bus_address_.empty()) {
    connection_ = dbus_bus_get_private(DBUS_BUS_SESSION, &error);
  } else {
    connection_ = dbus_connection_open_private(bus_address_.c_str(), &error);
    if (connection_ != nullptr && !dbus_bus_register(connection_, &error)) {
      return false;
    }
  }
  if (connection_ == nullptr) return false;
  dbus_connection_set_exit_on_disconnect(connection_, FALSE);
  return true;
}

bool WaylandPortalAppearanceMonitor::subscribe(DBusError& error) {
  dbus_bus_add_match(connection_, match_rule, &error);
  if (dbus_error_is_set(&error)) return false;
  filter_installed_ = dbus_connection_add_filter(
                          connection_, &WaylandPortalAppearanceMonitor::filter,
                          this, nullptr) != FALSE;
  return filter_installed_;
}

} // namespace cgpui
