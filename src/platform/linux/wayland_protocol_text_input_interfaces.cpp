#include "wayland_internal.hpp"

const wl_interface* zwp_text_input_manager_v3_get_text_input_types[]{
    &zwp_text_input_v3_interface,
    &wl_seat_interface,
};
const wl_message zwp_text_input_manager_v3_requests[]{
    {"destroy", "", nullptr},
    {
        "get_text_input",
        "no",
        zwp_text_input_manager_v3_get_text_input_types,
    },
};
const wl_interface zwp_text_input_manager_v3_interface{
    "zwp_text_input_manager_v3",
    1,
    2,
    zwp_text_input_manager_v3_requests,
    0,
    nullptr,
};

const wl_message zwp_text_input_v3_requests[]{
    {"destroy", "", nullptr},
    {"enable", "", nullptr},
    {"disable", "", nullptr},
    {"set_surrounding_text", "sii", nullptr},
    {"set_text_change_cause", "u", nullptr},
    {"set_content_type", "uu", nullptr},
    {"set_cursor_rectangle", "iiii", nullptr},
    {"commit", "", nullptr},
};
const wl_interface* zwp_text_input_v3_enter_types[]{
    &wl_surface_interface,
};
const wl_interface* zwp_text_input_v3_leave_types[]{
    &wl_surface_interface,
};
const wl_message zwp_text_input_v3_events[]{
    {"enter", "o", zwp_text_input_v3_enter_types},
    {"leave", "o", zwp_text_input_v3_leave_types},
    {"preedit_string", "sii", nullptr},
    {"commit_string", "s", nullptr},
    {"delete_surrounding_text", "uu", nullptr},
    {"done", "u", nullptr},
};
const wl_interface zwp_text_input_v3_interface{
    "zwp_text_input_v3",
    1,
    8,
    zwp_text_input_v3_requests,
    6,
    zwp_text_input_v3_events,
};
