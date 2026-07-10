#include "wayland_internal.hpp"

const wl_interface* zxdg_decoration_get_toplevel_types[]{
    &zxdg_toplevel_decoration_v1_interface,
    &xdg_toplevel_interface,
};
const wl_message zxdg_decoration_manager_requests[]{
    {"destroy", "", nullptr},
    {"get_toplevel_decoration", "no", zxdg_decoration_get_toplevel_types},
};
const wl_interface zxdg_decoration_manager_v1_interface{
    "zxdg_decoration_manager_v1", 1, 2,
    zxdg_decoration_manager_requests, 0, nullptr};

const wl_message zxdg_toplevel_decoration_requests[]{
    {"destroy", "", nullptr},
    {"set_mode", "u", nullptr},
    {"unset_mode", "", nullptr},
};
const wl_message zxdg_toplevel_decoration_events[]{
    {"configure", "u", nullptr},
};
const wl_interface zxdg_toplevel_decoration_v1_interface{
    "zxdg_toplevel_decoration_v1", 1, 3,
    zxdg_toplevel_decoration_requests, 1,
    zxdg_toplevel_decoration_events};
