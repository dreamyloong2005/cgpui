#pragma once

#include "cgpui/core/event_drag_drop.hpp"
#include "x11_internal.hpp"

namespace cgpui {

struct X11DragDropState {
  xcb_window_t source = XCB_WINDOW_NONE;
  xcb_atom_t target = XCB_ATOM_NONE;
  xcb_atom_t action = XCB_ATOM_NONE;
  Point position;
  DragDropPayload payload;
  bool payload_ready = false;
  bool entered = false;
  bool drop_pending = false;
};

}  // namespace cgpui
