#pragma once

#include "wayland_atspi_object_internal.hpp"

#include <array>
#include <cstdint>
#include <string_view>

struct DBusMessage;

namespace cgpui {

struct WaylandAtspiDbusReply;

struct WaylandAtspiRole {
  std::uint32_t id = 67;
  std::string_view name = "unknown";
};

[[nodiscard]] WaylandAtspiRole wayland_atspi_role_for(
    PlatformAccessibilityRole role);
[[nodiscard]] std::array<std::uint32_t, 2> wayland_atspi_states_for(
    const WaylandAtspiObjectNode& object);
[[nodiscard]] WaylandAtspiDbusReply wayland_atspi_role_state_reply_for(
    DBusMessage* request,
    const WaylandAtspiObjectNode& object);

} // namespace cgpui
