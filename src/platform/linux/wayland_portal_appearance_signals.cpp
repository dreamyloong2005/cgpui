#include "wayland_portal_appearance_monitor_internal.hpp"

#include <utility>

namespace cgpui {

int WaylandPortalAppearanceMonitor::descriptor() const {
  int fd = -1;
  if (connection_ == nullptr || !dbus_connection_get_unix_fd(connection_, &fd)) {
    return -1;
  }
  return fd;
}

bool WaylandPortalAppearanceMonitor::has_pending() const {
  return connection_ != nullptr &&
         dbus_connection_get_dispatch_status(connection_) ==
             DBUS_DISPATCH_DATA_REMAINS;
}

void WaylandPortalAppearanceMonitor::dispatch() {
  if (connection_ == nullptr) return;
  // Non-blocking: read what the socket has, then run the queued messages
  // (including signals queued while a blocking ReadOne was waiting).
  (void)dbus_connection_read_write(connection_, 0);
  while (connection_ != nullptr &&
         dbus_connection_dispatch(connection_) == DBUS_DISPATCH_DATA_REMAINS) {
  }
  if (connection_ != nullptr && !dbus_connection_get_is_connected(connection_)) {
    mark_unavailable("session bus disconnected");
    release_connection();
  }
}

DBusHandlerResult WaylandPortalAppearanceMonitor::filter(
    DBusConnection*, DBusMessage* message, void* data) {
  if (!dbus_message_is_signal(
          message, wayland_portal_settings_interface, "SettingChanged") ||
      !dbus_message_has_path(message, wayland_portal_path)) {
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }
  static_cast<WaylandPortalAppearanceMonitor*>(data)->handle_setting_changed(
      message);
  return DBUS_HANDLER_RESULT_HANDLED;
}

// Same contract as Windows: one callback per relevant signal, with `changed`
// telling whether the snapshot differs from the previous one.
void WaylandPortalAppearanceMonitor::handle_setting_changed(
    DBusMessage* message) {
  diagnostics_.signal_count += 1;
  WaylandPortalSettingSignal signal;
  SystemAppearance next = appearance_;
  const bool parsed = appearance_.supported &&
                      wayland_portal_parse_setting_changed(message, signal);
  const auto source = parsed
      ? wayland_portal_setting_source(signal.name_space, signal.key)
      : std::nullopt;
  if (!source || !wayland_portal_apply_setting(
                     next, signal.name_space, signal.key, signal.value)) {
    diagnostics_.ignored_signal_count += 1;
    return;
  }
  const bool changed = !(next == appearance_);
  appearance_ = std::move(next);
  if (!callback_) return;
  const SystemAppearanceCallback callback = callback_;
  callback(SystemAppearanceChange{
      .source = *source, .appearance = appearance_, .changed = changed});
}

} // namespace cgpui
