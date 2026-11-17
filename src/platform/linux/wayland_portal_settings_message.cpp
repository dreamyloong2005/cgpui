#include "wayland_portal_settings_internal.hpp"

namespace cgpui {
namespace {

bool next_of_type(DBusMessageIter& iterator, int type) {
  return dbus_message_iter_next(&iterator) &&
         dbus_message_iter_get_arg_type(&iterator) == type;
}

} // namespace

bool wayland_portal_apply_read_reply(
    SystemAppearance& appearance,
    std::string_view name_space,
    std::string_view key,
    DBusMessage* reply) {
  DBusMessageIter value;
  if (reply == nullptr || !dbus_message_iter_init(reply, &value) ||
      dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_VARIANT) {
    return false;
  }
  return wayland_portal_apply_setting(appearance, name_space, key, value);
}

bool wayland_portal_parse_setting_changed(
    DBusMessage* message,
    WaylandPortalSettingSignal& signal) {
  if (message == nullptr ||
      !dbus_message_is_signal(
          message, wayland_portal_settings_interface, "SettingChanged")) {
    return false;
  }
  DBusMessageIter iterator;
  if (!dbus_message_iter_init(message, &iterator) ||
      dbus_message_iter_get_arg_type(&iterator) != DBUS_TYPE_STRING) {
    return false;
  }
  const char* name_space = nullptr;
  dbus_message_iter_get_basic(&iterator, &name_space);
  if (!next_of_type(iterator, DBUS_TYPE_STRING)) return false;
  const char* key = nullptr;
  dbus_message_iter_get_basic(&iterator, &key);
  if (!next_of_type(iterator, DBUS_TYPE_VARIANT)) return false;
  signal.name_space = name_space;
  signal.key = key;
  signal.value = iterator;
  return true;
}

} // namespace cgpui
