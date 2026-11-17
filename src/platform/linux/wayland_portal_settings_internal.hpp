#pragma once

#include "cgpui/platform/platform_appearance.hpp"

#include <dbus/dbus.h>

#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

inline constexpr const char* wayland_portal_service =
    "org.freedesktop.portal.Desktop";
inline constexpr const char* wayland_portal_path =
    "/org/freedesktop/portal/desktop";
inline constexpr const char* wayland_portal_settings_interface =
    "org.freedesktop.portal.Settings";
inline constexpr const char* wayland_portal_appearance_namespace =
    "org.freedesktop.appearance";
inline constexpr const char* wayland_portal_gnome_interface_namespace =
    "org.gnome.desktop.interface";

// How a ReadOne/Read call ended, classified from the D-Bus error name.
enum class WaylandPortalReadStatus {
  value,
  not_found,
  unknown_method,
  unavailable,
};

struct WaylandPortalSettingSignal {
  std::string name_space;
  std::string key;
  DBusMessageIter value{};
};

[[nodiscard]] WaylandPortalReadStatus wayland_portal_classify_error(
    const char* error_name);
// Which change source a setting maps to; nullopt for settings CGPUI ignores.
[[nodiscard]] std::optional<SystemAppearanceChangeSource>
wayland_portal_setting_source(std::string_view name_space, std::string_view key);
// Applies one setting value to the snapshot. `value` points at a variant;
// nested variants (the deprecated Read wraps one more) are unwrapped.
// Returns false for unknown settings or a value of the wrong type, leaving
// the snapshot unchanged. A malformed or out-of-range accent clears it.
bool wayland_portal_apply_setting(
    SystemAppearance& appearance,
    std::string_view name_space,
    std::string_view key,
    DBusMessageIter value);
// Applies the body of a ReadOne (v) or Read (v wrapping v) reply.
bool wayland_portal_apply_read_reply(
    SystemAppearance& appearance,
    std::string_view name_space,
    std::string_view key,
    DBusMessage* reply);
// Parses SettingChanged (s namespace, s key, v value). The value iterator
// stays valid while `message` is alive.
[[nodiscard]] bool wayland_portal_parse_setting_changed(
    DBusMessage* message,
    WaylandPortalSettingSignal& signal);

} // namespace cgpui
