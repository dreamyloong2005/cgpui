#include "x11_window_internal.hpp"

namespace cgpui {

PlatformWindowLifecycleState X11Window::lifecycle_state() const {
  return PlatformWindowLifecycleState{
      .native_window_created = window_ != XCB_WINDOW_NONE,
      .initial_configure_complete = configured_,
      .active = active_,
      .focused = focused_,
      .close_requested = state_.close_requested,
      .display_state = display_state_,
  };
}

PlatformWindowCloseState X11Window::close_request_state() const {
  return close_controller_.state();
}

void X11Window::begin_close_request(WindowCloseRequestSource source) {
  if (!close_controller_.begin(source)) return;
  callback_(close_controller_.event());
}

void X11Window::request_close() {
  begin_close_request(WindowCloseRequestSource::application);
}

bool X11Window::resolve_close_request(
    PlatformWindowCloseResolution resolution) {
  if (!close_controller_.resolve(resolution)) return false;
  if (resolution == PlatformWindowCloseResolution::accept) {
    state_.close_requested = true;
  }
  return true;
}

void X11Window::send_net_wm_state(
    std::uint32_t action,
    xcb_atom_t first,
    xcb_atom_t second) {
  xcb_client_message_event_t event{};
  event.response_type = XCB_CLIENT_MESSAGE;
  event.format = 32;
  event.window = window_;
  event.type = atoms_.net_wm_state;
  event.data.data32[0] = action;
  event.data.data32[1] = first;
  event.data.data32[2] = second;
  event.data.data32[3] = 1;
  xcb_send_event(
      connection_, false, screen_->root,
      XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
          XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
      reinterpret_cast<const char*>(&event));
}

bool X11Window::request_display_state(
    PlatformWindowDisplayState display_state) {
  if (window_ == XCB_WINDOW_NONE) return false;
  if (display_state == PlatformWindowDisplayState::minimized) {
    xcb_unmap_window(connection_, window_);
  } else {
    xcb_map_window(connection_, window_);
    const bool fullscreen = display_state == PlatformWindowDisplayState::fullscreen;
    send_net_wm_state(
        fullscreen ? 1U : 0U, atoms_.net_wm_state_fullscreen, XCB_ATOM_NONE);
    const bool maximized = display_state == PlatformWindowDisplayState::maximized;
    send_net_wm_state(
        maximized ? 1U : 0U,
        atoms_.net_wm_state_maximized_horz,
        atoms_.net_wm_state_maximized_vert);
  }
  display_state_ = display_state;
  xcb_flush(connection_);
  return true;
}

PlatformWindowPositionState X11Window::position_state() const {
  return PlatformWindowPositionState{.supported = true, .position = position_};
}

bool X11Window::request_position(Point position) {
  if (window_ == XCB_WINDOW_NONE) return false;
  const std::uint32_t values[] = {
      static_cast<std::uint32_t>(static_cast<std::int32_t>(position.x)),
      static_cast<std::uint32_t>(static_cast<std::int32_t>(position.y)),
  };
  xcb_configure_window(
      connection_, window_, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
  xcb_flush(connection_);
  position_ = position;
  callback_(WindowMoved{.position = position});
  return true;
}

PlatformWindowChromeState X11Window::apply_window_chrome(
    WindowChromeOptions options) {
  chrome_ = options;
  struct MotifHints {
    std::uint32_t flags = 2;
    std::uint32_t functions = 0;
    std::uint32_t decorations = 1;
    std::int32_t input_mode = 0;
    std::uint32_t status = 0;
  } hints;
  hints.decorations = options.decorations && options.titlebar_visible ? 1U : 0U;
  if (window_ != XCB_WINDOW_NONE) {
    xcb_change_property(
        connection_, XCB_PROP_MODE_REPLACE, window_, atoms_.motif_wm_hints,
        atoms_.motif_wm_hints, 32, 5, &hints);
    xcb_flush(connection_);
  }
  WindowChromeOptions applied = options;
  applied.transparent_background = false;
  return PlatformWindowChromeState{
      .supported = true,
      .decoration_control_supported = true,
      .transparency_supported = false,
      .backend = "x11-motif-hints",
      .requested = options,
      .applied = applied,
      .reason = options.transparent_background
          ? "X11 transparency requires a compositing visual"
          : std::string{},
  };
}

}  // namespace cgpui
