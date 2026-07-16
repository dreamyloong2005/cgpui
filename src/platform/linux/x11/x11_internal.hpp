#pragma once

#include "cgpui/platform/platform.hpp"

#include <xcb/xcb.h>

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
