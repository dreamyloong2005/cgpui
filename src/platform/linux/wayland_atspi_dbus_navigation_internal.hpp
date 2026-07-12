#pragma once

#include "wayland_atspi_object_internal.hpp"

#include <dbus/dbus.h>

#include <string_view>

namespace cgpui {

struct WaylandAtspiDbusReply;

[[nodiscard]] WaylandAtspiDbusReply wayland_atspi_navigation_reply_for(
    DBusMessage* request,
    const WaylandAtspiObjectNode& object,
    std::string_view bus_name,
    std::string_view root_object_path);

} // namespace cgpui
