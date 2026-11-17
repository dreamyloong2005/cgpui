#include "wayland_portal_appearance_monitor_internal.hpp"

#include <array>
#include <utility>

namespace cgpui {
namespace {

// The portal is D-Bus activated; allow for a cold start without blocking the
// UI thread for libdbus's 25 s default.
constexpr int read_timeout_ms = 2000;

struct SettingName {
  const char* name_space;
  const char* key;
};

// color-scheme comes first: if it cannot be read the portal is unusable.
constexpr std::array<SettingName, 4> appearance_settings{{
    {wayland_portal_appearance_namespace, "color-scheme"},
    {wayland_portal_appearance_namespace, "accent-color"},
    {wayland_portal_appearance_namespace, "contrast"},
    {wayland_portal_gnome_interface_namespace, "enable-animations"},
}};

} // namespace

bool WaylandPortalAppearanceMonitor::read_snapshot() {
  SystemAppearance snapshot{
      .supported = true,
      .backend = "wayland",
      .mode = SystemAppearanceMode::light,
      .accent_color_available = false,
      .accent_color = Color{},
      .transparency_effects_enabled = false,
      .client_area_animation_enabled = true,
      .high_contrast = false,
      .unsupported_reason = {},
  };
  for (std::size_t index = 0; index < appearance_settings.size(); ++index) {
    const SettingName setting = appearance_settings[index];
    const WaylandPortalReadStatus status =
        read_setting(snapshot, setting.name_space, setting.key);
    // Later settings that fail are treated as absent and keep defaults.
    if (status == WaylandPortalReadStatus::unavailable && index == 0) {
      return false;
    }
  }
  appearance_ = std::move(snapshot);
  return true;
}

WaylandPortalReadStatus WaylandPortalAppearanceMonitor::read_setting(
    SystemAppearance& snapshot, const char* name_space, const char* key) {
  // ReadOne first; portals older than version 2 only have the deprecated
  // Read, which is remembered after the first UnknownMethod.
  for (int attempt = 0; attempt < 2; ++attempt) {
    const char* method = use_deprecated_read_ ? "Read" : "ReadOne";
    DBusMessage* request = dbus_message_new_method_call(
        wayland_portal_service, wayland_portal_path,
        wayland_portal_settings_interface, method);
    if (request == nullptr ||
        !dbus_message_append_args(
            request, DBUS_TYPE_STRING, &name_space, DBUS_TYPE_STRING, &key,
            DBUS_TYPE_INVALID)) {
      if (request != nullptr) dbus_message_unref(request);
      last_error_ = "out of memory building the portal request";
      return WaylandPortalReadStatus::unavailable;
    }
    if (use_deprecated_read_) diagnostics_.deprecated_read_count += 1;
    DBusError error;
    dbus_error_init(&error);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(
        connection_, request, read_timeout_ms, &error);
    dbus_message_unref(request);
    const WaylandPortalReadStatus status = wayland_portal_classify_error(
        dbus_error_is_set(&error) ? error.name : nullptr);
    if (status == WaylandPortalReadStatus::unknown_method ||
        status == WaylandPortalReadStatus::unavailable) {
      last_error_ = std::string(error.name) + ": " +
                    (error.message == nullptr ? "" : error.message);
    }
    if (dbus_error_is_set(&error)) dbus_error_free(&error);
    if (reply != nullptr) {
      if (wayland_portal_apply_read_reply(snapshot, name_space, key, reply)) {
        diagnostics_.setting_read_count += 1;
      }
      dbus_message_unref(reply);
    }
    if (status != WaylandPortalReadStatus::unknown_method) return status;
    if (use_deprecated_read_) break;
    use_deprecated_read_ = true;
  }
  return WaylandPortalReadStatus::unavailable;
}

void WaylandPortalAppearanceMonitor::mark_unavailable(std::string reason) {
  diagnostics_.unavailable_reason = reason;
  appearance_ = SystemAppearance{
      .supported = false,
      .backend = "wayland",
      .mode = SystemAppearanceMode::light,
      .unsupported_reason = std::move(reason),
  };
}

void WaylandPortalAppearanceMonitor::release_connection() {
  diagnostics_.connected = false;
  if (connection_ == nullptr) return;
  if (filter_installed_) {
    dbus_connection_remove_filter(
        connection_, &WaylandPortalAppearanceMonitor::filter, this);
    filter_installed_ = false;
  }
  dbus_connection_close(connection_);
  dbus_connection_unref(connection_);
  connection_ = nullptr;
}

} // namespace cgpui
