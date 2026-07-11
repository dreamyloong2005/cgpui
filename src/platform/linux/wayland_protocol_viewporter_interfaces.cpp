#include "wayland_internal.hpp"

const wl_interface* wp_viewporter_get_viewport_types[]{
    &wp_viewport_interface,
    &wl_surface_interface,
};
const wl_message wp_viewporter_requests[]{
    {"destroy", "", nullptr},
    {"get_viewport", "no", wp_viewporter_get_viewport_types},
};
const wl_interface wp_viewporter_interface{
    "wp_viewporter", 1, 2, wp_viewporter_requests, 0, nullptr};

const wl_message wp_viewport_requests[]{
    {"destroy", "", nullptr},
    {"set_source", "ffff", nullptr},
    {"set_destination", "ii", nullptr},
};
const wl_interface wp_viewport_interface{
    "wp_viewport", 1, 3, wp_viewport_requests, 0, nullptr};
