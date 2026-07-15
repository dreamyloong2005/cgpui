#include "macos_window_internal.hpp"

namespace cgpui {

PlatformWindowCloseState MacOSWindow::close_request_state() const {
  return close_controller_.state();
}

bool MacOSWindow::resolve_close_request(PlatformWindowCloseResolution resolution) {
  if (!close_controller_.resolve(resolution)) return false;
  if (resolution == PlatformWindowCloseResolution::cancel) {
    state_.close_requested = false;
  } else if (resolution == PlatformWindowCloseResolution::accept) {
    if (delegate_ != nil) [delegate_ detachWindow];
    [window_ setDelegate:nil];
    [window_ orderOut:nil];
    [window_ close];
    window_ = nil;
    active_ = false;
    focused_ = false;
    minimized_ = false;
  }
  return true;
}

void MacOSWindow::close_requested(WindowCloseRequestSource source) {
  if (!close_controller_.begin(source)) return;
  state_.close_requested = true;
  callback_(close_controller_.event());
}

PlatformWindowLifecycleState MacOSWindow::lifecycle_state() const {
  PlatformWindowDisplayState display_state = PlatformWindowDisplayState::normal;
  if (window_ != nil && [window_ isMiniaturized]) {
    display_state = PlatformWindowDisplayState::minimized;
  } else if (window_ != nil &&
             ([window_ styleMask] & NSWindowStyleMaskFullScreen) != 0) {
    display_state = PlatformWindowDisplayState::fullscreen;
  } else if (window_ != nil && [window_ isZoomed]) {
    display_state = PlatformWindowDisplayState::maximized;
  }
  return PlatformWindowLifecycleState{
      .native_window_created = window_ != nil,
      .initial_configure_complete = window_ != nil,
      .active = active_,
      .focused = focused_,
      .close_requested = state_.close_requested,
      .display_state = display_state};
}

bool MacOSWindow::request_display_state(PlatformWindowDisplayState display_state) {
  if (window_ == nil) return false;
  switch (display_state) {
    case PlatformWindowDisplayState::normal:
      [window_ deminiaturize:nil];
      return true;
    case PlatformWindowDisplayState::minimized:
      [window_ miniaturize:nil];
      return true;
    case PlatformWindowDisplayState::maximized:
      [window_ zoom:nil];
      return true;
    case PlatformWindowDisplayState::fullscreen:
      [window_ toggleFullScreen:nil];
      return true;
  }
  return false;
}

PlatformWindowPositionState MacOSWindow::position_state() const {
  if (window_ == nil) return {};
  const NSPoint origin = [window_ frame].origin;
  return PlatformWindowPositionState{
      .supported = true,
      .position = Point{static_cast<float>(origin.x), static_cast<float>(origin.y)}};
}

bool MacOSWindow::request_position(Point position) {
  if (window_ == nil || minimized_) return false;
  NSRect frame = [window_ frame];
  frame.origin = NSMakePoint(position.x, position.y);
  [window_ setFrameOrigin:frame.origin];
  return true;
}

void MacOSWindow::resized() {
  refresh_state();
  callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
}

void MacOSWindow::moved() {
  const auto position = position_state().position;
  if (position.has_value()) callback_(WindowMoved{.position = *position});
}

void MacOSWindow::activated(bool active) {
  active_ = active;
  callback_(WindowActivated{.active = active});
}

void MacOSWindow::focused(bool focused) {
  focused_ = focused;
  callback_(WindowFocused{.focused = focused});
}

void MacOSWindow::minimized(bool minimized) {
  minimized_ = minimized;
  if (minimized) callback_(WindowMinimized{.minimized = true});
  else callback_(WindowRestored{});
}

}  // namespace cgpui
