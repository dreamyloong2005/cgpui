#include "macos_window_internal.hpp"

namespace cgpui {

void MacOSWindow::set_cursor(CursorShape cursor_shape) {
  NSCursor* cursor = [NSCursor arrowCursor];
  switch (cursor_shape) {
    case CursorShape::default_arrow: cursor = [NSCursor arrowCursor]; break;
    case CursorShape::pointing_hand: cursor = [NSCursor pointingHandCursor]; break;
    case CursorShape::text: cursor = [NSCursor IBeamCursor]; break;
    case CursorShape::crosshair: cursor = [NSCursor crosshairCursor]; break;
    case CursorShape::resize_left_right: cursor = [NSCursor resizeLeftRightCursor]; break;
    case CursorShape::resize_up_down: cursor = [NSCursor resizeUpDownCursor]; break;
    case CursorShape::resize_north_west_south_east: cursor = [NSCursor closedHandCursor]; break;
    case CursorShape::resize_north_east_south_west: cursor = [NSCursor closedHandCursor]; break;
    case CursorShape::resize_all: cursor = [NSCursor openHandCursor]; break;
    case CursorShape::wait: cursor = [NSCursor disappearingItemCursor]; break;
    case CursorShape::progress: cursor = [NSCursor operationNotAllowedCursor]; break;
    case CursorShape::help: cursor = [NSCursor contextualMenuCursor]; break;
    case CursorShape::up_arrow: cursor = [NSCursor arrowCursor]; break;
    case CursorShape::not_allowed: cursor = [NSCursor operationNotAllowedCursor]; break;
  }
  [cursor set];
}

}  // namespace cgpui
