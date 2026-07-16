#include "x11_window_internal.hpp"

namespace cgpui {
namespace {
xcb_window_t event_window(const xcb_generic_event_t& event) {
  switch (event.response_type & 0x7f) {
    case XCB_EXPOSE:
      return reinterpret_cast<const xcb_expose_event_t&>(event).window;
    case XCB_CONFIGURE_NOTIFY:
      return reinterpret_cast<const xcb_configure_notify_event_t&>(event).window;
    case XCB_FOCUS_IN:
    case XCB_FOCUS_OUT:
      return reinterpret_cast<const xcb_focus_in_event_t&>(event).event;
    case XCB_MAP_NOTIFY:
      return reinterpret_cast<const xcb_map_notify_event_t&>(event).window;
    case XCB_UNMAP_NOTIFY:
      return reinterpret_cast<const xcb_unmap_notify_event_t&>(event).window;
    case XCB_CLIENT_MESSAGE:
      return reinterpret_cast<const xcb_client_message_event_t&>(event).window;
    case XCB_ENTER_NOTIFY:
    case XCB_LEAVE_NOTIFY:
      return reinterpret_cast<const xcb_enter_notify_event_t&>(event).event;
    case XCB_MOTION_NOTIFY:
      return reinterpret_cast<const xcb_motion_notify_event_t&>(event).event;
    case XCB_BUTTON_PRESS:
    case XCB_BUTTON_RELEASE:
      return reinterpret_cast<const xcb_button_press_event_t&>(event).event;
    case XCB_KEY_PRESS:
    case XCB_KEY_RELEASE:
      return reinterpret_cast<const xcb_key_press_event_t&>(event).event;
    default:
      return XCB_WINDOW_NONE;
  }
}
}  // namespace

bool X11Window::owns_event(const xcb_generic_event_t& event) const {
  return window_ != XCB_WINDOW_NONE && event_window(event) == window_;
}

void X11Window::handle_event(const xcb_generic_event_t& event) {
  const std::uint8_t type = event.response_type & 0x7f;
  if (type == XCB_MOTION_NOTIFY || type == XCB_ENTER_NOTIFY ||
      type == XCB_LEAVE_NOTIFY || type == XCB_BUTTON_PRESS ||
      type == XCB_BUTTON_RELEASE) {
    handle_pointer_event(event);
    return;
  }
  if (type == XCB_KEY_PRESS || type == XCB_KEY_RELEASE) {
    handle_keyboard_event(event);
    return;
  }
  switch (type) {
    case XCB_EXPOSE:
      callback_(WindowRedrawRequested{});
      break;
    case XCB_CONFIGURE_NOTIFY: {
      const auto& configure =
          reinterpret_cast<const xcb_configure_notify_event_t&>(event);
      const Size size{
          static_cast<float>(configure.width),
          static_cast<float>(configure.height)};
      if (size.width != state_.framebuffer_size.width ||
          size.height != state_.framebuffer_size.height) {
        state_.framebuffer_size = size;
        callback_(WindowResized{.size = size, .scale = state_.scale});
      }
      position_ = Point{
          static_cast<float>(configure.x), static_cast<float>(configure.y)};
      break;
    }
    case XCB_FOCUS_IN:
    case XCB_FOCUS_OUT:
      focused_ = (event.response_type & 0x7f) == XCB_FOCUS_IN;
      active_ = focused_;
      callback_(WindowFocused{.focused = focused_});
      callback_(WindowActivated{.active = active_});
      break;
    case XCB_MAP_NOTIFY:
      if (display_state_ == PlatformWindowDisplayState::minimized) {
        display_state_ = PlatformWindowDisplayState::normal;
        callback_(WindowRestored{});
      }
      break;
    case XCB_UNMAP_NOTIFY:
      display_state_ = PlatformWindowDisplayState::minimized;
      callback_(WindowMinimized{.minimized = true});
      break;
    case XCB_CLIENT_MESSAGE: {
      const auto& message =
          reinterpret_cast<const xcb_client_message_event_t&>(event);
      if (message.type == atoms_.wm_protocols && message.format == 32 &&
          message.data.data32[0] == atoms_.wm_delete_window) {
        begin_close_request(WindowCloseRequestSource::window_manager);
      }
      break;
    }
    default:
      break;
  }
}

}  // namespace cgpui
