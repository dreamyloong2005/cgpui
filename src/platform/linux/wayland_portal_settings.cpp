#include "wayland_portal_settings_internal.hpp"

#include <array>
#include <cmath>
#include <cstdint>

namespace cgpui {
namespace {

// Settings values arrive in one variant (ReadOne, SettingChanged) or two
// (the deprecated Read). Unwrap however many there are, within reason.
void unwrap_variants(DBusMessageIter& value) {
  for (int depth = 0; depth < 4; ++depth) {
    if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_VARIANT) return;
    DBusMessageIter inner;
    dbus_message_iter_recurse(&value, &inner);
    value = inner;
  }
}

std::optional<std::uint32_t> read_uint32(DBusMessageIter value) {
  if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_UINT32) {
    return std::nullopt;
  }
  dbus_uint32_t number = 0;
  dbus_message_iter_get_basic(&value, &number);
  return number;
}

std::optional<bool> read_boolean(DBusMessageIter value) {
  if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_BOOLEAN) {
    return std::nullopt;
  }
  dbus_bool_t flag = FALSE;
  dbus_message_iter_get_basic(&value, &flag);
  return flag != FALSE;
}

bool in_unit_range(double component) {
  return std::isfinite(component) && component >= 0.0 && component <= 1.0;
}

// accent-color is (ddd) sRGB in [0, 1]. Anything else means "no accent".
bool apply_accent(SystemAppearance& appearance, DBusMessageIter value) {
  appearance.accent_color_available = false;
  appearance.accent_color = Color{};
  if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_STRUCT) return true;
  DBusMessageIter field;
  dbus_message_iter_recurse(&value, &field);
  std::array<double, 3> rgb{};
  for (double& component : rgb) {
    if (dbus_message_iter_get_arg_type(&field) != DBUS_TYPE_DOUBLE) return true;
    dbus_message_iter_get_basic(&field, &component);
    if (!in_unit_range(component)) return true;
    dbus_message_iter_next(&field);
  }
  appearance.accent_color = Color{
      static_cast<float>(rgb[0]),
      static_cast<float>(rgb[1]),
      static_cast<float>(rgb[2]),
      1.0F};
  appearance.accent_color_available = true;
  return true;
}

} // namespace

WaylandPortalReadStatus wayland_portal_classify_error(const char* error_name) {
  if (error_name == nullptr) return WaylandPortalReadStatus::value;
  const std::string_view name(error_name);
  if (name == DBUS_ERROR_UNKNOWN_METHOD) {
    return WaylandPortalReadStatus::unknown_method;
  }
  // The portal answered but has no such setting (Error.NotFound and kin).
  if (name.starts_with("org.freedesktop.portal.Error.")) {
    return WaylandPortalReadStatus::not_found;
  }
  return WaylandPortalReadStatus::unavailable;
}

std::optional<SystemAppearanceChangeSource> wayland_portal_setting_source(
    std::string_view name_space, std::string_view key) {
  if (name_space == wayland_portal_appearance_namespace) {
    if (key == "color-scheme") return SystemAppearanceChangeSource::color_set;
    if (key == "accent-color") {
      return SystemAppearanceChangeSource::colorization;
    }
    if (key == "contrast") return SystemAppearanceChangeSource::high_contrast;
  }
  if (name_space == wayland_portal_gnome_interface_namespace &&
      key == "enable-animations") {
    return SystemAppearanceChangeSource::client_area_animation;
  }
  return std::nullopt;
}

bool wayland_portal_apply_setting(
    SystemAppearance& appearance,
    std::string_view name_space,
    std::string_view key,
    DBusMessageIter value) {
  const auto source = wayland_portal_setting_source(name_space, key);
  if (!source) return false;
  unwrap_variants(value);
  if (*source == SystemAppearanceChangeSource::colorization) {
    return apply_accent(appearance, value);
  }
  if (*source == SystemAppearanceChangeSource::client_area_animation) {
    const std::optional<bool> enabled = read_boolean(value);
    if (!enabled) return false;
    appearance.client_area_animation_enabled = *enabled;
    return true;
  }
  const std::optional<std::uint32_t> number = read_uint32(value);
  if (!number) return false;
  if (*source == SystemAppearanceChangeSource::high_contrast) {
    appearance.high_contrast = *number == 1;
    return true;
  }
  // color-scheme: 1 prefers dark; 0 (no preference), 2 (prefers light),
  // and unknown values are light.
  appearance.mode =
      *number == 1 ? SystemAppearanceMode::dark : SystemAppearanceMode::light;
  return true;
}

} // namespace cgpui
