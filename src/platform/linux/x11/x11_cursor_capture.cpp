#include "x11_window_internal.hpp"

namespace cgpui {
namespace {

const char* cursor_name(CursorShape shape) {
  switch (shape) {
    case CursorShape::pointing_hand: return "pointer";
    case CursorShape::text: return "text";
    case CursorShape::crosshair: return "crosshair";
    case CursorShape::resize_left_right: return "ew-resize";
    case CursorShape::resize_up_down: return "ns-resize";
    case CursorShape::resize_north_west_south_east: return "nwse-resize";
    case CursorShape::resize_north_east_south_west: return "nesw-resize";
    case CursorShape::resize_all: return "move";
    case CursorShape::wait: return "wait";
    case CursorShape::progress: return "progress";
    case CursorShape::help: return "help";
    case CursorShape::up_arrow: return "up-arrow";
    case CursorShape::not_allowed: return "not-allowed";
    default: return "default";
  }
}

}  // namespace

void X11Window::set_cursor(CursorShape cursor_shape) {
  cursor_shape_ = cursor_shape;
  if (window_ == XCB_WINDOW_NONE || cursor_context_ == nullptr) return;
  const xcb_cursor_t replacement =
      xcb_cursor_load_cursor(cursor_context_, cursor_name(cursor_shape));
  if (replacement == XCB_CURSOR_NONE) return;
  const std::uint32_t value = replacement;
  xcb_change_window_attributes(connection_, window_, XCB_CW_CURSOR, &value);
  if (cursor_ != XCB_CURSOR_NONE) xcb_free_cursor(connection_, cursor_);
  cursor_ = replacement;
  xcb_flush(connection_);
}

PlatformPointerCaptureState X11Window::pointer_capture_state() const {
  return PlatformPointerCaptureState{
      .supported = true,
      .captured = pointer_captured_,
  };
}

void X11Window::set_pointer_capture(bool captured) {
  if (window_ == XCB_WINDOW_NONE || captured == pointer_captured_) return;
  bool applied = !captured;
  if (captured) {
    const auto cookie = xcb_grab_pointer(
        connection_, true, window_,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
            XCB_EVENT_MASK_POINTER_MOTION,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
        XCB_WINDOW_NONE, XCB_CURSOR_NONE, XCB_CURRENT_TIME);
    xcb_grab_pointer_reply_t* reply =
        xcb_grab_pointer_reply(connection_, cookie, nullptr);
    applied = reply != nullptr && reply->status == XCB_GRAB_STATUS_SUCCESS;
    std::free(reply);
  } else {
    xcb_ungrab_pointer(connection_, XCB_CURRENT_TIME);
  }
  if (!applied) return;
  pointer_captured_ = captured;
  xcb_flush(connection_);
  callback_(PointerCaptureChanged{.captured = captured});
}

}  // namespace cgpui
