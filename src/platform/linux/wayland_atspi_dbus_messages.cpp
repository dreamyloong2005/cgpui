#include "wayland_atspi_dbus_messages_internal.hpp"
#include "wayland_atspi_dbus_navigation_internal.hpp"
#include "wayland_atspi_role_state_internal.hpp"

#include <array>
#include <string_view>

namespace cgpui {
namespace {

constexpr std::string_view introspection_xml = R"xml(<node>
  <interface name="org.freedesktop.DBus.Introspectable">
    <method name="Introspect"><arg direction="out" type="s"/></method>
  </interface>
  <interface name="org.a11y.atspi.Accessible">
    <property name="Parent" type="(so)" access="read"/>
    <property name="ChildCount" type="i" access="read"/>
    <method name="GetInterfaces"><arg direction="out" type="as"/></method>
    <method name="GetChildAtIndex"><arg direction="in" type="i"/><arg direction="out" type="(so)"/></method>
    <method name="GetChildren"><arg direction="out" type="a(so)"/></method>
    <method name="GetIndexInParent"><arg direction="out" type="i"/></method>
    <method name="GetApplication"><arg direction="out" type="(so)"/></method>
    <method name="GetRole"><arg direction="out" type="u"/></method>
    <method name="GetRoleName"><arg direction="out" type="s"/></method>
    <method name="GetLocalizedRoleName"><arg direction="out" type="s"/></method>
    <method name="GetState"><arg direction="out" type="au"/></method>
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

WaylandAtspiDbusReply wayland_atspi_dbus_reply_for(
    DBusMessage* request,
    const WaylandAtspiObjectNode& object,
    std::string_view bus_name,
    std::string_view root_object_path) {
  if (dbus_message_is_method_call(
          request, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
    const char* xml = introspection_xml.data();
    return {.handled = true, .message = string_reply(request, xml)};
  }
  if (dbus_message_is_method_call(
          request, "org.a11y.atspi.Accessible", "GetInterfaces")) {
    return {.handled = true, .message = interface_reply(request)};
  }
  WaylandAtspiDbusReply navigation = wayland_atspi_navigation_reply_for(
      request, object, bus_name, root_object_path);
  if (navigation.handled) return navigation;
  return wayland_atspi_role_state_reply_for(request, object);
}

} // namespace cgpui
