#pragma once

#include <dbus/dbus.h>

#include <functional>
#include <initializer_list>

// Builders for portal Settings messages used by parser and fake-portal tests.
namespace cgpui::test {

using PortalValueWriter = std::function<void(DBusMessageIter*)>;

struct PortalValue {
  const char* signature = "u";
  PortalValueWriter write;
};

// Appends `value` in a variant; `wrapped` adds the deprecated Read's extra
// outer variant.
inline void append_portal_value(
    DBusMessageIter* iterator, const PortalValue& value, bool wrapped) {
  DBusMessageIter outer;
  if (wrapped) {
    dbus_message_iter_open_container(iterator, DBUS_TYPE_VARIANT, "v", &outer);
  }
  DBusMessageIter* target = wrapped ? &outer : iterator;
  DBusMessageIter variant;
  dbus_message_iter_open_container(
      target, DBUS_TYPE_VARIANT, value.signature, &variant);
  value.write(&variant);
  dbus_message_iter_close_container(target, &variant);
  if (wrapped) dbus_message_iter_close_container(iterator, &outer);
}

inline PortalValue portal_uint32(dbus_uint32_t value) {
  return {"u", [value](DBusMessageIter* iterator) {
    dbus_message_iter_append_basic(iterator, DBUS_TYPE_UINT32, &value);
  }};
}

inline PortalValue portal_boolean(bool value) {
  return {"b", [value](DBusMessageIter* iterator) {
    const dbus_bool_t flag = value ? TRUE : FALSE;
    dbus_message_iter_append_basic(iterator, DBUS_TYPE_BOOLEAN, &flag);
  }};
}

inline PortalValue portal_rgb(double r, double g, double b) {
  return {"(ddd)", [r, g, b](DBusMessageIter* iterator) {
    DBusMessageIter fields;
    dbus_message_iter_open_container(
        iterator, DBUS_TYPE_STRUCT, nullptr, &fields);
    for (const double component : {r, g, b}) {
      dbus_message_iter_append_basic(&fields, DBUS_TYPE_DOUBLE, &component);
    }
    dbus_message_iter_close_container(iterator, &fields);
  }};
}

inline PortalValue portal_string(const char* value) {
  return {"s", [value](DBusMessageIter* iterator) {
    dbus_message_iter_append_basic(iterator, DBUS_TYPE_STRING, &value);
  }};
}

// A ReadOne (wrapped = false) or Read (wrapped = true) reply body.
inline DBusMessage* make_read_reply(const PortalValue& value, bool wrapped) {
  DBusMessage* reply = dbus_message_new(DBUS_MESSAGE_TYPE_METHOD_RETURN);
  DBusMessageIter iterator;
  dbus_message_iter_init_append(reply, &iterator);
  append_portal_value(&iterator, value, wrapped);
  return reply;
}

inline DBusMessage* make_setting_changed(
    const char* name_space, const char* key, const PortalValue& value) {
  DBusMessage* signal = dbus_message_new_signal(
      "/org/freedesktop/portal/desktop", "org.freedesktop.portal.Settings",
      "SettingChanged");
  DBusMessageIter iterator;
  dbus_message_iter_init_append(signal, &iterator);
  dbus_message_iter_append_basic(&iterator, DBUS_TYPE_STRING, &name_space);
  dbus_message_iter_append_basic(&iterator, DBUS_TYPE_STRING, &key);
  append_portal_value(&iterator, value, false);
  return signal;
}

} // namespace cgpui::test
