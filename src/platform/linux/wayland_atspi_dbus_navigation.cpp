#include "wayland_atspi_dbus_navigation_internal.hpp"

#include "wayland_atspi_dbus_messages_internal.hpp"

#include <cstdint>

namespace cgpui {
namespace {

constexpr const char* accessible_interface = "org.a11y.atspi.Accessible";
constexpr const char* null_object_path = "/org/a11y/atspi/null";

bool append_object_reference(
    DBusMessageIter& parent,
    std::string_view bus_name,
    std::string_view object_path) {
  DBusMessageIter structure;
  if (!dbus_message_iter_open_container(
          &parent, DBUS_TYPE_STRUCT, nullptr, &structure)) {
    return false;
  }
  const char* bus = bus_name.data();
  const char* path = object_path.data();
  return dbus_message_iter_append_basic(
             &structure, DBUS_TYPE_STRING, &bus) &&
         dbus_message_iter_append_basic(
             &structure, DBUS_TYPE_OBJECT_PATH, &path) &&
         dbus_message_iter_close_container(&parent, &structure);
}

DBusMessage* object_reply(
    DBusMessage* request,
    std::string_view bus_name,
    std::string_view object_path) {
  DBusMessage* reply = dbus_message_new_method_return(request);
  if (reply == nullptr) return nullptr;
  DBusMessageIter root;
  dbus_message_iter_init_append(reply, &root);
  if (!append_object_reference(root, bus_name, object_path)) {
    dbus_message_unref(reply);
    return nullptr;
  }
  return reply;
}

DBusMessage* object_variant_reply(
    DBusMessage* request,
    std::string_view bus_name,
    std::string_view object_path) {
  DBusMessage* reply = dbus_message_new_method_return(request);
  if (reply == nullptr) return nullptr;
  DBusMessageIter root;
  DBusMessageIter variant;
  dbus_message_iter_init_append(reply, &root);
  if (!dbus_message_iter_open_container(
          &root, DBUS_TYPE_VARIANT, "(so)", &variant) ||
      !append_object_reference(variant, bus_name, object_path) ||
      !dbus_message_iter_close_container(&root, &variant)) {
    dbus_message_unref(reply);
    return nullptr;
  }
  return reply;
}

DBusMessage* int32_reply(
    DBusMessage* request, std::int32_t value, bool variant) {
  DBusMessage* reply = dbus_message_new_method_return(request);
  if (reply == nullptr) return nullptr;
  DBusMessageIter root;
  dbus_message_iter_init_append(reply, &root);
  if (!variant) {
    if (dbus_message_iter_append_basic(&root, DBUS_TYPE_INT32, &value)) {
      return reply;
    }
  } else {
    DBusMessageIter container;
    if (dbus_message_iter_open_container(
            &root, DBUS_TYPE_VARIANT, DBUS_TYPE_INT32_AS_STRING, &container) &&
        dbus_message_iter_append_basic(
            &container, DBUS_TYPE_INT32, &value) &&
        dbus_message_iter_close_container(&root, &container)) {
      return reply;
    }
  }
  dbus_message_unref(reply);
  return nullptr;
}

DBusMessage* children_reply(
    DBusMessage* request,
    std::string_view bus_name,
    const std::vector<std::string>& paths) {
  DBusMessage* reply = dbus_message_new_method_return(request);
  if (reply == nullptr) return nullptr;
  DBusMessageIter root;
  DBusMessageIter array;
  dbus_message_iter_init_append(reply, &root);
  if (!dbus_message_iter_open_container(
          &root, DBUS_TYPE_ARRAY, "(so)", &array)) {
    dbus_message_unref(reply);
    return nullptr;
  }
  for (const std::string& path : paths) {
    if (!append_object_reference(array, bus_name, path)) {
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

bool property_request(DBusMessage* request, const char*& property_name) {
  const char* interface_name = nullptr;
  DBusError error;
  dbus_error_init(&error);
  const bool parsed = dbus_message_get_args(
      request,
      &error,
      DBUS_TYPE_STRING, &interface_name,
      DBUS_TYPE_STRING, &property_name,
      DBUS_TYPE_INVALID);
  dbus_error_free(&error);
  return parsed && interface_name != nullptr &&
         std::string_view(interface_name) == accessible_interface;
}

} // namespace

WaylandAtspiDbusReply wayland_atspi_navigation_reply_for(
    DBusMessage* request,
    const WaylandAtspiObjectNode& object,
    std::string_view bus_name,
    std::string_view root_object_path) {
  if (dbus_message_is_method_call(
          request, DBUS_INTERFACE_PROPERTIES, "Get")) {
    const char* property_name = nullptr;
    if (!property_request(request, property_name)) return {};
    if (std::string_view(property_name) == "Parent") {
      std::string_view parent = null_object_path;
      if (object.parent_object_path.has_value()) {
        parent = *object.parent_object_path;
      }
      return {true, object_variant_reply(request, bus_name, parent)};
    }
    if (std::string_view(property_name) == "ChildCount") {
      return {true, int32_reply(
          request,
          static_cast<std::int32_t>(object.child_object_paths.size()),
          true)};
    }
    return {};
  }
  if (dbus_message_is_method_call(
          request, accessible_interface, "GetChildAtIndex")) {
    std::int32_t index = -1;
    DBusError error;
    dbus_error_init(&error);
    const bool parsed = dbus_message_get_args(
        request, &error, DBUS_TYPE_INT32, &index, DBUS_TYPE_INVALID);
    dbus_error_free(&error);
    std::string_view path = null_object_path;
    if (parsed && index >= 0 &&
        static_cast<std::size_t>(index) < object.child_object_paths.size()) {
      path = object.child_object_paths[static_cast<std::size_t>(index)];
    }
    return {true, object_reply(request, bus_name, path)};
  }
  if (dbus_message_is_method_call(
          request, accessible_interface, "GetChildren")) {
    return {true, children_reply(
        request, bus_name, object.child_object_paths)};
  }
  if (dbus_message_is_method_call(
          request, accessible_interface, "GetIndexInParent")) {
    return {true, int32_reply(request, object.index_in_parent, false)};
  }
  if (dbus_message_is_method_call(
          request, accessible_interface, "GetApplication")) {
    const std::string_view path = root_object_path.empty()
        ? null_object_path
        : root_object_path;
    return {true, object_reply(request, bus_name, path)};
  }
  return {};
}

} // namespace cgpui
