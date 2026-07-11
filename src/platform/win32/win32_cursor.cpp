#include "win32_cursor_internal.hpp"

namespace cgpui {

const wchar_t* win32_system_cursor_id(CursorShape cursor_shape) {
  switch (cursor_shape) {
    case CursorShape::pointing_hand:
      return MAKEINTRESOURCEW(32649);
    case CursorShape::text:
      return MAKEINTRESOURCEW(32513);
    case CursorShape::crosshair:
      return MAKEINTRESOURCEW(32515);
    case CursorShape::resize_left_right:
      return MAKEINTRESOURCEW(32644);
    case CursorShape::resize_up_down:
      return MAKEINTRESOURCEW(32645);
    case CursorShape::resize_north_west_south_east:
      return MAKEINTRESOURCEW(32642);
    case CursorShape::resize_north_east_south_west:
      return MAKEINTRESOURCEW(32643);
    case CursorShape::resize_all:
      return MAKEINTRESOURCEW(32646);
    case CursorShape::wait:
      return MAKEINTRESOURCEW(32514);
    case CursorShape::progress:
      return MAKEINTRESOURCEW(32650);
    case CursorShape::help:
      return MAKEINTRESOURCEW(32651);
    case CursorShape::up_arrow:
      return MAKEINTRESOURCEW(32516);
    case CursorShape::not_allowed:
      return MAKEINTRESOURCEW(32648);
    case CursorShape::default_arrow:
      return MAKEINTRESOURCEW(32512);
  }
  return MAKEINTRESOURCEW(32512);
}

HCURSOR load_win32_system_cursor(CursorShape cursor_shape) {
  HCURSOR cursor = LoadCursorW(nullptr, win32_system_cursor_id(cursor_shape));
  if (cursor == nullptr && cursor_shape != CursorShape::default_arrow) {
    cursor = LoadCursorW(
        nullptr,
        win32_system_cursor_id(CursorShape::default_arrow));
  }
  return cursor;
}

} // namespace cgpui
