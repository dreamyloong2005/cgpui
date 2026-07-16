#pragma once

#include "cgpui/platform/platform.hpp"

#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace cgpui {

struct X11Atoms {
  xcb_atom_t wm_protocols = XCB_ATOM_NONE;
  xcb_atom_t wm_delete_window = XCB_ATOM_NONE;
  xcb_atom_t utf8_string = XCB_ATOM_NONE;
  xcb_atom_t net_wm_name = XCB_ATOM_NONE;
  xcb_atom_t net_wm_state = XCB_ATOM_NONE;
  xcb_atom_t net_wm_state_fullscreen = XCB_ATOM_NONE;
  xcb_atom_t net_wm_state_maximized_horz = XCB_ATOM_NONE;
  xcb_atom_t net_wm_state_maximized_vert = XCB_ATOM_NONE;
  xcb_atom_t motif_wm_hints = XCB_ATOM_NONE;
  xcb_atom_t xdnd_aware = XCB_ATOM_NONE;
  xcb_atom_t xdnd_enter = XCB_ATOM_NONE;
  xcb_atom_t xdnd_position = XCB_ATOM_NONE;
  xcb_atom_t xdnd_status = XCB_ATOM_NONE;
  xcb_atom_t xdnd_leave = XCB_ATOM_NONE;
  xcb_atom_t xdnd_drop = XCB_ATOM_NONE;
  xcb_atom_t xdnd_finished = XCB_ATOM_NONE;
  xcb_atom_t xdnd_selection = XCB_ATOM_NONE;
  xcb_atom_t xdnd_type_list = XCB_ATOM_NONE;
  xcb_atom_t xdnd_action_copy = XCB_ATOM_NONE;
  xcb_atom_t xdnd_action_move = XCB_ATOM_NONE;
  xcb_atom_t xdnd_transfer = XCB_ATOM_NONE;
  xcb_atom_t text_plain_utf8 = XCB_ATOM_NONE;
  xcb_atom_t uri_list = XCB_ATOM_NONE;
};

[[nodiscard]] Error x11_error(ErrorCode code, std::string message);
[[nodiscard]] xcb_screen_t* x11_screen(
    xcb_connection_t* connection,
    int screen_index);
[[nodiscard]] Result<xcb_atom_t> x11_intern_atom(
    xcb_connection_t* connection,
    std::string_view name);
[[nodiscard]] Result<X11Atoms> x11_load_atoms(xcb_connection_t* connection);

}  // namespace cgpui
