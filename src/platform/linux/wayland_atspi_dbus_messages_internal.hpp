#pragma once

#include <dbus/dbus.h>

namespace cgpui {

struct WaylandAtspiDbusReply {
  bool handled = false;
  DBusMessage* message = nullptr;
};

[[nodiscard]] WaylandAtspiDbusReply wayland_atspi_dbus_reply_for(
    DBusMessage* request);

} // namespace cgpui
