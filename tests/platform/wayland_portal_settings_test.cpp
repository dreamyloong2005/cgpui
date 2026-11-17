#include "wayland_portal_settings_internal.hpp"
#include "wayland_portal_test_messages.hpp"

#include <cmath>
#include <iostream>

namespace {

using cgpui::SystemAppearance;
using cgpui::SystemAppearanceChangeSource;
using cgpui::SystemAppearanceMode;
using cgpui::WaylandPortalReadStatus;
using namespace cgpui::test;

constexpr const char* appearance = "org.freedesktop.appearance";
constexpr const char* gnome = "org.gnome.desktop.interface";

int fail(int code, const char* message) {
  std::cerr << "wayland portal settings: " << message << '\n';
  return code;
}

// Applies a constructed ReadOne/Read reply to `snapshot`.
bool apply(
    SystemAppearance& snapshot,
    const char* name_space,
    const char* key,
    const PortalValue& value,
    bool wrapped = false) {
  DBusMessage* reply = make_read_reply(value, wrapped);
  const bool applied =
      cgpui::wayland_portal_apply_read_reply(snapshot, name_space, key, reply);
  dbus_message_unref(reply);
  return applied;
}

bool near(float lhs, double rhs) {
  return std::fabs(lhs - static_cast<float>(rhs)) < 0.001F;
}

int check_color_scheme() {
  SystemAppearance snapshot;
  if (!apply(snapshot, appearance, "color-scheme", portal_uint32(1)) ||
      snapshot.mode != SystemAppearanceMode::dark) {
    return fail(1, "color-scheme 1 must be dark");
  }
  for (const dbus_uint32_t light : {0U, 2U, 7U}) {
    snapshot.mode = SystemAppearanceMode::dark;
    if (!apply(snapshot, appearance, "color-scheme", portal_uint32(light)) ||
        snapshot.mode != SystemAppearanceMode::light) {
      return fail(2, "color-scheme 0, 2, and unknown values must be light");
    }
  }
  if (!apply(snapshot, appearance, "color-scheme", portal_uint32(1), true) ||
      snapshot.mode != SystemAppearanceMode::dark) {
    return fail(3, "deprecated Read double variant must be unwrapped");
  }
  if (apply(snapshot, appearance, "color-scheme", portal_string("dark")) ||
      snapshot.mode != SystemAppearanceMode::dark) {
    return fail(4, "wrong value type must be rejected without a change");
  }
  return 0;
}

int check_accent() {
  SystemAppearance snapshot;
  if (!apply(snapshot, appearance, "accent-color", portal_rgb(0.2, 0.4, 0.6)) ||
      !snapshot.accent_color_available || !near(snapshot.accent_color.r, 0.2) ||
      !near(snapshot.accent_color.g, 0.4) || !near(snapshot.accent_color.b, 0.6) ||
      !near(snapshot.accent_color.a, 1.0)) {
    return fail(5, "in-range accent must be reported with alpha 1");
  }
  if (!apply(snapshot, appearance, "accent-color", portal_rgb(1.5, 0.0, 0.0)) ||
      snapshot.accent_color_available) {
    return fail(6, "out-of-range accent must clear the accent");
  }
  snapshot.accent_color_available = true;
  if (!apply(snapshot, appearance, "accent-color", portal_rgb(-1, -1, -1)) ||
      snapshot.accent_color_available) {
    return fail(7, "unset (-1) accent must clear the accent");
  }
  snapshot.accent_color_available = true;
  if (!apply(snapshot, appearance, "accent-color", portal_uint32(3)) ||
      snapshot.accent_color_available) {
    return fail(8, "malformed accent must clear the accent");
  }
  return 0;
}

int check_contrast_and_motion() {
  SystemAppearance snapshot;
  if (!apply(snapshot, appearance, "contrast", portal_uint32(1)) ||
      !snapshot.high_contrast) {
    return fail(9, "contrast 1 must be high contrast");
  }
  if (!apply(snapshot, appearance, "contrast", portal_uint32(0)) ||
      snapshot.high_contrast) {
    return fail(10, "contrast 0 must clear high contrast");
  }
  if (!apply(snapshot, gnome, "enable-animations", portal_boolean(false)) ||
      snapshot.client_area_animation_enabled) {
    return fail(11, "enable-animations false must disable animation");
  }
  if (apply(snapshot, appearance, "reduced-motion", portal_uint32(1)) ||
      apply(snapshot, gnome, "color-scheme", portal_uint32(1)) ||
      snapshot.mode != SystemAppearanceMode::light) {
    return fail(12, "unknown settings must be ignored");
  }
  return 0;
}

int check_mapping() {
  using cgpui::wayland_portal_setting_source;
  if (wayland_portal_setting_source(appearance, "color-scheme") !=
          SystemAppearanceChangeSource::color_set ||
      wayland_portal_setting_source(appearance, "accent-color") !=
          SystemAppearanceChangeSource::colorization ||
      wayland_portal_setting_source(appearance, "contrast") !=
          SystemAppearanceChangeSource::high_contrast ||
      wayland_portal_setting_source(gnome, "enable-animations") !=
          SystemAppearanceChangeSource::client_area_animation ||
      wayland_portal_setting_source(gnome, "gtk-theme").has_value()) {
    return fail(13, "setting to change source mapping is wrong");
  }
  using cgpui::wayland_portal_classify_error;
  if (wayland_portal_classify_error(nullptr) != WaylandPortalReadStatus::value ||
      wayland_portal_classify_error("org.freedesktop.portal.Error.NotFound") !=
          WaylandPortalReadStatus::not_found ||
      wayland_portal_classify_error(DBUS_ERROR_UNKNOWN_METHOD) !=
          WaylandPortalReadStatus::unknown_method ||
      wayland_portal_classify_error(DBUS_ERROR_SERVICE_UNKNOWN) !=
          WaylandPortalReadStatus::unavailable ||
      wayland_portal_classify_error(DBUS_ERROR_NO_REPLY) !=
          WaylandPortalReadStatus::unavailable) {
    return fail(14, "D-Bus error classification is wrong");
  }
  return 0;
}

int check_signal() {
  DBusMessage* signal =
      make_setting_changed(appearance, "color-scheme", portal_uint32(1));
  cgpui::WaylandPortalSettingSignal parsed;
  SystemAppearance snapshot;
  const bool ok = cgpui::wayland_portal_parse_setting_changed(signal, parsed) &&
                  parsed.name_space == appearance &&
                  parsed.key == "color-scheme" &&
                  cgpui::wayland_portal_apply_setting(
                      snapshot, parsed.name_space, parsed.key, parsed.value) &&
                  snapshot.mode == SystemAppearanceMode::dark;
  dbus_message_unref(signal);
  if (!ok) return fail(15, "SettingChanged must parse and apply");
  DBusMessage* other = dbus_message_new_signal(
      "/org/freedesktop/portal/desktop", "org.freedesktop.portal.Settings",
      "Other");
  DBusMessage* empty = dbus_message_new_signal(
      "/org/freedesktop/portal/desktop", "org.freedesktop.portal.Settings",
      "SettingChanged");
  const bool rejected =
      !cgpui::wayland_portal_parse_setting_changed(other, parsed) &&
      !cgpui::wayland_portal_parse_setting_changed(empty, parsed) &&
      !cgpui::wayland_portal_parse_setting_changed(nullptr, parsed);
  dbus_message_unref(other);
  dbus_message_unref(empty);
  return rejected ? 0 : fail(16, "malformed signals must be rejected");
}

} // namespace

int main() {
  for (const auto check : {&check_color_scheme, &check_accent,
                           &check_contrast_and_motion, &check_mapping,
                           &check_signal}) {
    if (const int code = check(); code != 0) return code;
  }
  return 0;
}
