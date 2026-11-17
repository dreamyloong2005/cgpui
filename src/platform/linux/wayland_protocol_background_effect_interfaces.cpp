#include "wayland_protocol_background_effect_internal.hpp"

const wl_interface* ext_background_effect_get_types[]{
    &ext_background_effect_surface_v1_interface,
    &wl_surface_interface,
};
const wl_interface* ext_background_effect_region_types[]{
    &wl_region_interface,
};
const wl_message ext_background_effect_manager_requests[]{
    {"destroy", "", nullptr},
    {"get_background_effect", "no", ext_background_effect_get_types},
};
const wl_message ext_background_effect_manager_events[]{
    {"capabilities", "u", nullptr},
};
const wl_interface ext_background_effect_manager_v1_interface{
    "ext_background_effect_manager_v1", 1, 2,
    ext_background_effect_manager_requests, 1,
    ext_background_effect_manager_events};

const wl_message ext_background_effect_surface_requests[]{
    {"destroy", "", nullptr},
    {"set_blur_region", "?o", ext_background_effect_region_types},
};
const wl_interface ext_background_effect_surface_v1_interface{
    "ext_background_effect_surface_v1", 1, 2,
    ext_background_effect_surface_requests, 0, nullptr};

const wl_interface* org_kde_kwin_blur_create_types[]{
    &org_kde_kwin_blur_interface,
    &wl_surface_interface,
};
const wl_interface* org_kde_kwin_blur_surface_types[]{
    &wl_surface_interface,
};
const wl_interface* org_kde_kwin_blur_region_types[]{
    &wl_region_interface,
};
const wl_message org_kde_kwin_blur_manager_requests[]{
    {"create", "no", org_kde_kwin_blur_create_types},
    {"unset", "o", org_kde_kwin_blur_surface_types},
};
const wl_interface org_kde_kwin_blur_manager_interface{
    "org_kde_kwin_blur_manager", 1, 2, org_kde_kwin_blur_manager_requests, 0,
    nullptr};

const wl_message org_kde_kwin_blur_requests[]{
    {"commit", "", nullptr},
    {"set_region", "?o", org_kde_kwin_blur_region_types},
    {"release", "", nullptr},
};
const wl_interface org_kde_kwin_blur_interface{
    "org_kde_kwin_blur", 1, 3, org_kde_kwin_blur_requests, 0, nullptr};
