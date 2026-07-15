#include "macos_window_internal.hpp"

namespace cgpui {

PlatformWindowCloseState MacOSWindow::close_request_state() const {
  return close_controller_.state();
}

bool MacOSWindow::resolve_close_request(PlatformWindowCloseResolution resolution) {
  if (!close_controller_.resolve(resolution)) return false;
  if (resolution == PlatformWindowCloseResolution::cancel) {
    state_.close_requested = false;
  }
  return true;
}

void MacOSWindow::close_requested(WindowCloseRequestSource source) {
  if (!close_controller_.begin(source)) return;
  state_.close_requested = true;
  callback_(close_controller_.event());
}

PlatformPointerCaptureState MacOSWindow::pointer_capture_state() const {
  return {};
}

void MacOSWindow::set_pointer_capture(bool captured) {
  (void)captured;
}

PlatformWindowChromeState MacOSWindow::apply_window_chrome(WindowChromeOptions options) {
  chrome_ = options;
  return PlatformWindowChromeState{
      .supported = true,
      .decoration_control_supported = true,
      .transparency_supported = true,
      .backend = "cocoa",
      .requested = options,
      .applied = options};
}

void MacOSWindow::update_accessibility_tree(PlatformAccessibilityTreeUpdate update) {
  (void)update;
}

PlatformWindowLifecycleState MacOSWindow::lifecycle_state() const {
  return PlatformWindowLifecycleState{
      .native_window_created = window_ != nil && [window_ isVisible],
      .initial_configure_complete = window_ != nil,
      .active = active_,
      .focused = focused_,
      .close_requested = state_.close_requested,
      .display_state = minimized_
          ? PlatformWindowDisplayState::minimized
          : PlatformWindowDisplayState::normal};
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
