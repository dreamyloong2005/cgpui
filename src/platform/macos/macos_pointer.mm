#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"

#include <algorithm>

namespace cgpui {

Point macos_event_position_in_framebuffer(
    double x,
    double y,
    double view_height,
    float scale) {
  const double normalized_scale = scale > 0.0F ? scale : 1.0;
  const double framebuffer_x = x * normalized_scale;
  const double framebuffer_y = (view_height - y) * normalized_scale;
  return Point{
      static_cast<float>(framebuffer_x / normalized_scale),
      static_cast<float>(framebuffer_y / normalized_scale)};
}

MouseButton macos_mouse_button(std::uint64_t button_number) {
  switch (button_number) {
    case 0: return MouseButton::left;
    case 1: return MouseButton::right;
    case 2: return MouseButton::middle;
    case 3: return MouseButton::back;
    case 4: return MouseButton::forward;
    default: return MouseButton::other;
  }
}

void MacOSWindow::pointer_moved(NSEvent* event) {
  callback_(PointerMoved{.position = event_position(event)});
}

void MacOSWindow::pointer_exited(NSEvent* event) {
  callback_(PointerExited{.position = event_position(event)});
}

void MacOSWindow::pointer_button(NSEvent* event, bool pressed) {
  const auto clicks = std::clamp<NSInteger>([event clickCount], 1, 255);
  callback_(PointerButton{
      .button = macos_mouse_button([event buttonNumber]),
      .pressed = pressed,
      .click_count = static_cast<std::uint8_t>(clicks),
      .position = event_position(event),
  });
}

PlatformPointerCaptureState MacOSWindow::pointer_capture_state() const {
  return {.supported = true, .captured = pointer_captured_};
}

void MacOSWindow::set_pointer_capture(bool captured) {
  if (captured == pointer_captured_) return;
  if (!captured) {
    release_pointer_capture(true);
    return;
  }
  __weak CGPUIMacOSContentView* content = content_view_;
  __weak NSWindow* native_window = window_;
  const NSEventMask mask = NSEventMaskMouseMoved |
      NSEventMaskLeftMouseDragged | NSEventMaskRightMouseDragged |
      NSEventMaskOtherMouseDragged | NSEventMaskLeftMouseDown |
      NSEventMaskLeftMouseUp | NSEventMaskRightMouseDown |
      NSEventMaskRightMouseUp | NSEventMaskOtherMouseDown |
      NSEventMaskOtherMouseUp;
  pointer_monitor_ = [NSEvent addLocalMonitorForEventsMatchingMask:mask
      handler:^NSEvent*(NSEvent* event) {
        CGPUIMacOSContentView* target = content;
        NSWindow* target_window = native_window;
        if (target != nil && target_window != nil && [event window] != target_window) {
          [target dispatchCapturedMouseEvent:event];
        }
        return event;
      }];
  pointer_captured_ = pointer_monitor_ != nil;
  callback_(PointerCaptureChanged{.captured = pointer_captured_});
}

void MacOSWindow::release_pointer_capture(bool notify) {
  if (pointer_monitor_ != nil) {
    [NSEvent removeMonitor:pointer_monitor_];
    pointer_monitor_ = nil;
  }
  if (!pointer_captured_) return;
  pointer_captured_ = false;
  if (notify) callback_(PointerCaptureChanged{.captured = false});
}

Point MacOSWindow::event_position(NSEvent* event) const {
  if (event == nil || content_view_ == nil) return {};
  NSPoint point;
  if ([event window] == window_) {
    point = [event locationInWindow];
  } else {
    NSPoint screen = [[event window] convertPointToScreen:[event locationInWindow]];
    point = [window_ convertPointFromScreen:screen];
  }
  return macos_event_position_in_framebuffer(
      point.x,
      point.y,
      NSHeight([content_view_ bounds]),
      state_.scale.value);
}

}  // namespace cgpui
