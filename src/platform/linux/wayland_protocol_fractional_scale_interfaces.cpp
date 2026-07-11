#include "wayland_internal.hpp"

const wl_interface* wp_fractional_scale_manager_get_types[]{
    &wp_fractional_scale_v1_interface,
    &wl_surface_interface,
};
const wl_message wp_fractional_scale_manager_requests[]{
    {"destroy", "", nullptr},
    {"get_fractional_scale", "no", wp_fractional_scale_manager_get_types},
};
const wl_interface wp_fractional_scale_manager_v1_interface{
    "wp_fractional_scale_manager_v1", 1, 2,
    wp_fractional_scale_manager_requests, 0, nullptr};

const wl_message wp_fractional_scale_requests[]{
    {"destroy", "", nullptr},
};
const wl_message wp_fractional_scale_events[]{
    {"preferred_scale", "u", nullptr},
};
const wl_interface wp_fractional_scale_v1_interface{
    "wp_fractional_scale_v1", 1, 1, wp_fractional_scale_requests, 1,
    wp_fractional_scale_events};
