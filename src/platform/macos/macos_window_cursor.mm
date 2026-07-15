#include "macos_window_internal.hpp"
#include "macos_input_internal.hpp"

namespace cgpui {

NSCursor* macos_cursor_for_shape(CursorShape cursor_shape) {
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
  return cursor;
}

void MacOSWindow::set_cursor(CursorShape cursor_shape) {
  cursor_shape_ = cursor_shape;
  NSCursor* cursor = macos_cursor_for_shape(cursor_shape);
  [cursor set];
  if (window_ != nil && content_view_ != nil) {
    [window_ invalidateCursorRectsForView:content_view_];
  }
}

}  // namespace cgpui
