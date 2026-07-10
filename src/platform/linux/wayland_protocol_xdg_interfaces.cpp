#include "wayland_internal.hpp"

const wl_interface xdg_positioner_interface{
    "xdg_positioner", 1, 0, nullptr, 0, nullptr};

const wl_interface* xdg_toplevel_set_parent_types[]{&xdg_toplevel_interface};
const wl_interface* xdg_toplevel_show_window_menu_types[]{
    &wl_seat_interface, nullptr, nullptr, nullptr};
const wl_interface* xdg_toplevel_move_types[]{&wl_seat_interface, nullptr};
const wl_interface* xdg_toplevel_resize_types[]{
    &wl_seat_interface, nullptr, nullptr};
const wl_interface* xdg_toplevel_set_fullscreen_types[]{&wl_output_interface};
const wl_message xdg_toplevel_requests[]{
    {"destroy", "", nullptr},
    {"set_parent", "?o", xdg_toplevel_set_parent_types},
    {"set_title", "s", nullptr},
    {"set_app_id", "s", nullptr},
    {"show_window_menu", "ouii", xdg_toplevel_show_window_menu_types},
    {"move", "ou", xdg_toplevel_move_types},
    {"resize", "ouu", xdg_toplevel_resize_types},
    {"set_max_size", "ii", nullptr},
    {"set_min_size", "ii", nullptr},
    {"set_maximized", "", nullptr},
    {"unset_maximized", "", nullptr},
    {"set_fullscreen", "?o", xdg_toplevel_set_fullscreen_types},
    {"unset_fullscreen", "", nullptr},
    {"set_minimized", "", nullptr},
};
const wl_message xdg_toplevel_events[]{
    {"configure", "iia", nullptr},
    {"close", "", nullptr},
};
const wl_interface xdg_toplevel_interface{
    "xdg_toplevel",
    1,
    14,
    xdg_toplevel_requests,
    2,
    xdg_toplevel_events,
};

const wl_interface* xdg_surface_get_toplevel_types[]{
    &xdg_toplevel_interface,
};
const wl_message xdg_surface_requests[]{
    {"destroy", "", nullptr},
    {"get_toplevel", "n", xdg_surface_get_toplevel_types},
    {"get_popup", "noo", nullptr},
    {"set_window_geometry", "iiii", nullptr},
    {"ack_configure", "u", nullptr},
};
const wl_message xdg_surface_events[]{
    {"configure", "u", nullptr},
};
const wl_interface xdg_surface_interface{
    "xdg_surface",
    1,
    5,
    xdg_surface_requests,
    1,
    xdg_surface_events,
};

const wl_interface* xdg_wm_base_create_positioner_types[]{
    &xdg_positioner_interface,
};
const wl_interface* xdg_wm_base_get_xdg_surface_types[]{
    &xdg_surface_interface,
    &wl_surface_interface,
};
const wl_message xdg_wm_base_requests[]{
    {"destroy", "", nullptr},
    {"create_positioner", "n", xdg_wm_base_create_positioner_types},
    {"get_xdg_surface", "no", xdg_wm_base_get_xdg_surface_types},
    {"pong", "u", nullptr},
};
const wl_message xdg_wm_base_events[]{
    {"ping", "u", nullptr},
};
const wl_interface xdg_wm_base_interface{
    "xdg_wm_base",
    1,
    4,
    xdg_wm_base_requests,
    1,
    xdg_wm_base_events,
};
