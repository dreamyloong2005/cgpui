#pragma once

#include "wayland_atspi_dbus_test_support.hpp"

#include <cstdint>
#include <string>

namespace cgpui::test {

struct AtspiEventPayload {
  std::string detail;
  std::int32_t detail1 = 0;
  std::int32_t detail2 = 0;
  std::string value;
  AtspiObjectReference source;
};

inline AtspiEventPayload read_atspi_event_payload(DBusMessage* message) {
  DBusMessageIter root;
  dbus_message_iter_init(message, &root);
  AtspiEventPayload payload;
  const char* detail = nullptr;
  dbus_message_iter_get_basic(&root, &detail);
  payload.detail = detail == nullptr ? "" : detail;
  dbus_message_iter_next(&root);
  dbus_message_iter_get_basic(&root, &payload.detail1);
  dbus_message_iter_next(&root);
  dbus_message_iter_get_basic(&root, &payload.detail2);
  dbus_message_iter_next(&root);
  DBusMessageIter variant;
  dbus_message_iter_recurse(&root, &variant);
  const char* value = nullptr;
  dbus_message_iter_get_basic(&variant, &value);
  payload.value = value == nullptr ? "" : value;
  dbus_message_iter_next(&root);
  payload.source = read_object_reference(&root);
  return payload;
}

} // namespace cgpui::test
