#include "wayland_atspi_role_state_internal.hpp"

#include "wayland_atspi_dbus_messages_internal.hpp"

#include <dbus/dbus.h>

namespace cgpui {
namespace {

constexpr const char* accessible_interface = "org.a11y.atspi.Accessible";

void set_state(std::array<std::uint32_t, 2>& words, std::uint32_t state) {
  words[state / 32] |= 1U << (state % 32);
}

DBusMessage* uint32_reply(DBusMessage* request, std::uint32_t value) {
  DBusMessage* reply = dbus_message_new_method_return(request);
  if (reply == nullptr || !dbus_message_append_args(
          reply, DBUS_TYPE_UINT32, &value, DBUS_TYPE_INVALID)) {
    if (reply != nullptr) dbus_message_unref(reply);
    return nullptr;
  }
  return reply;
}

DBusMessage* string_reply(DBusMessage* request, std::string_view value) {
  DBusMessage* reply = dbus_message_new_method_return(request);
  const char* text = value.data();
  if (reply == nullptr || !dbus_message_append_args(
          reply, DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID)) {
    if (reply != nullptr) dbus_message_unref(reply);
    return nullptr;
  }
  return reply;
}

DBusMessage* state_reply(
    DBusMessage* request, const std::array<std::uint32_t, 2>& words) {
  DBusMessage* reply = dbus_message_new_method_return(request);
  if (reply == nullptr) return nullptr;
  DBusMessageIter root;
  DBusMessageIter array;
  dbus_message_iter_init_append(reply, &root);
  if (!dbus_message_iter_open_container(
          &root, DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32_AS_STRING, &array)) {
    dbus_message_unref(reply);
    return nullptr;
  }
  for (const std::uint32_t word : words) {
    if (!dbus_message_iter_append_basic(
            &array, DBUS_TYPE_UINT32, &word)) {
      dbus_message_unref(reply);
      return nullptr;
    }
  }
  if (!dbus_message_iter_close_container(&root, &array)) {
    dbus_message_unref(reply);
    return nullptr;
  }
  return reply;
}

} // namespace

WaylandAtspiRole wayland_atspi_role_for(PlatformAccessibilityRole role) {
  switch (role) {
    case PlatformAccessibilityRole::label: return {29, "label"};
    case PlatformAccessibilityRole::button: return {43, "push button"};
    case PlatformAccessibilityRole::text: return {61, "text"};
    case PlatformAccessibilityRole::text_input: return {79, "entry"};
    case PlatformAccessibilityRole::image: return {27, "image"};
    case PlatformAccessibilityRole::checkbox: return {7, "check box"};
    case PlatformAccessibilityRole::radio: return {44, "radio button"};
    case PlatformAccessibilityRole::switch_control:
      return {62, "toggle button"};
    case PlatformAccessibilityRole::slider: return {51, "slider"};
    case PlatformAccessibilityRole::list_item: return {32, "list item"};
    case PlatformAccessibilityRole::menu_item: return {35, "menu item"};
    case PlatformAccessibilityRole::generic: return {67, "unknown"};
  }
  return {67, "unknown"};
}

std::array<std::uint32_t, 2> wayland_atspi_states_for(
    const WaylandAtspiObjectNode& object) {
  std::array<std::uint32_t, 2> words{};
  set_state(words, 25); // showing
  set_state(words, 30); // visible
  if (object.enabled) {
    set_state(words, 8);  // enabled
    set_state(words, 24); // sensitive
  }
  if (object.focusable) set_state(words, 11);
  if (object.focused) {
    set_state(words, 1);  // active
    set_state(words, 12); // focused
  }
  if (object.patterns.toggled.has_value()) {
    set_state(words, 41); // checkable
    if (*object.patterns.toggled) set_state(words, 4);
  }
  if (object.patterns.value_settable ||
      object.role == PlatformAccessibilityRole::text_input) {
    set_state(words, 7); // editable
  }
  return words;
}

WaylandAtspiDbusReply wayland_atspi_role_state_reply_for(
    DBusMessage* request, const WaylandAtspiObjectNode& object) {
  const WaylandAtspiRole role = wayland_atspi_role_for(object.role);
  if (dbus_message_is_method_call(
          request, accessible_interface, "GetRole")) {
    return {true, uint32_reply(request, role.id)};
  }
  if (dbus_message_is_method_call(
          request, accessible_interface, "GetRoleName") ||
      dbus_message_is_method_call(
          request, accessible_interface, "GetLocalizedRoleName")) {
    return {true, string_reply(request, role.name)};
  }
  if (dbus_message_is_method_call(
          request, accessible_interface, "GetState")) {
    return {true, state_reply(request, wayland_atspi_states_for(object))};
  }
  return {};
}

} // namespace cgpui
