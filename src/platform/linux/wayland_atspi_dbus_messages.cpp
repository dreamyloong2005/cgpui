#include "wayland_atspi_dbus_messages_internal.hpp"

#include <array>
#include <string_view>

namespace cgpui {
namespace {

constexpr std::string_view introspection_xml = R"xml(<node>
  <interface name="org.freedesktop.DBus.Introspectable">
    <method name="Introspect"><arg direction="out" type="s"/></method>
  </interface>
  <interface name="org.a11y.atspi.Accessible">
    <method name="GetInterfaces"><arg direction="out" type="as"/></method>
  </interface>
  <interface name="org.a11y.atspi.Component"/>
</node>)xml";

constexpr std::array<const char*, 2> exposed_interfaces{
    "org.a11y.atspi.Accessible",
    "org.a11y.atspi.Component",
};

DBusMessage* string_reply(DBusMessage* request, const char* value) {
  DBusMessage* reply = dbus_message_new_method_return(request);
  if (reply == nullptr ||
      !dbus_message_append_args(
          reply, DBUS_TYPE_STRING, &value, DBUS_TYPE_INVALID)) {
    if (reply != nullptr) dbus_message_unref(reply);
    return nullptr;
  }
  return reply;
}

DBusMessage* interface_reply(DBusMessage* request) {
  DBusMessage* reply = dbus_message_new_method_return(request);
  if (reply == nullptr) return nullptr;
  DBusMessageIter root;
  DBusMessageIter array;
  dbus_message_iter_init_append(reply, &root);
  if (!dbus_message_iter_open_container(
          &root, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array)) {
    dbus_message_unref(reply);
    return nullptr;
  }
  for (const char* interface_name : exposed_interfaces) {
    if (!dbus_message_iter_append_basic(
            &array, DBUS_TYPE_STRING, &interface_name)) {
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

WaylandAtspiDbusReply wayland_atspi_dbus_reply_for(DBusMessage* request) {
  if (dbus_message_is_method_call(
          request, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
    const char* xml = introspection_xml.data();
    return {.handled = true, .message = string_reply(request, xml)};
  }
  if (dbus_message_is_method_call(
          request, "org.a11y.atspi.Accessible", "GetInterfaces")) {
    return {.handled = true, .message = interface_reply(request)};
  }
  return {};
}

} // namespace cgpui
