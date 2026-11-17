#pragma once

#include "cgpui/core/event_pointer.hpp"

#include <cstdint>

struct wl_seat;

namespace cgpui {

class WaylandWindow;

// Runs the client title bar behaviors (move, resize, window menu, caption
// buttons) for a pointer button the window has already dispatched.
void wayland_window_title_bar_pointer_button(
    WaylandWindow& window, wl_seat* seat, std::uint32_t serial,
    std::uint32_t time, MouseButton button, bool pressed, Point position);

} // namespace cgpui
