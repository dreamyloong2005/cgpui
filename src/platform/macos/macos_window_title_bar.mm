#include "macos_window_title_bar_internal.hpp"
#include "macos_window_internal.hpp"

namespace cgpui {
namespace {

bool region_contains(Rect rect, Point point) {
  return point.x >= rect.origin.x && point.y >= rect.origin.y &&
         point.x < rect.origin.x + rect.size.width &&
         point.y < rect.origin.y + rect.size.height;
}

bool is_button(WindowControlArea area) {
  return area == WindowControlArea::minimize_button ||
         area == WindowControlArea::maximize_button ||
         area == WindowControlArea::close_button;
}

}  // namespace

std::optional<WindowControlArea> macos_window_control_area_at(
    const std::vector<WindowControlRegion>& regions,
    Point point) {
  for (auto region = regions.rbegin(); region != regions.rend(); ++region) {
    if (region_contains(region->bounds, point)) return region->area;
  }
  return std::nullopt;
}

MacOSTitleBarDoubleClick macos_title_bar_double_click_action(
    NSString* preference) {
  if (preference == nil || [preference isEqualToString:@"Maximize"]) {
    return MacOSTitleBarDoubleClick::zoom;
  }
  if ([preference isEqualToString:@"Minimize"]) {
    return MacOSTitleBarDoubleClick::minimize;
  }
  return MacOSTitleBarDoubleClick::none;
}

void MacOSWindow::set_window_control_regions(
    std::vector<WindowControlRegion> regions) {
  control_regions_ = std::move(regions);
}

// The app has already received the event. Only the left button acts, as on
// the other backends: a caption press drags (or zooms/minimizes on a double
// click), and a release over the button region the press started on runs
// that button.
void MacOSWindow::title_bar_pointer_button(
    NSEvent* event, Point position, bool pressed) {
  if (!client_title_bar_ || window_ == nil || [event buttonNumber] != 0) return;
  const std::optional<WindowControlArea> area =
      macos_window_control_area_at(control_regions_, position);
  if (!pressed) {
    const std::optional<WindowControlArea> started = pressed_control_;
    pressed_control_.reset();
    if (!started || area != started) return;
    if (*started == WindowControlArea::close_button) {
      close_requested(WindowCloseRequestSource::window_manager);
    } else {
      (void)perform_window_action(PlatformWindowActionRequest{
          .action = *started == WindowControlArea::minimize_button
              ? PlatformWindowAction::minimize
              : PlatformWindowAction::toggle_maximize});
    }
    return;
  }
  pressed_control_ = area && is_button(*area) ? area : std::nullopt;
  if (area != WindowControlArea::caption) return;
  if ([event clickCount] == 2) {
    switch (macos_title_bar_double_click_action([[NSUserDefaults standardUserDefaults]
                stringForKey:@"AppleActionOnDoubleClick"])) {
      case MacOSTitleBarDoubleClick::zoom: [window_ zoom:nil]; break;
      case MacOSTitleBarDoubleClick::minimize: [window_ miniaturize:nil]; break;
      case MacOSTitleBarDoubleClick::none: break;
    }
    return;
  }
  [window_ performWindowDragWithEvent:event];
}

PlatformWindowFrameMetrics MacOSWindow::frame_metrics() const {
  PlatformWindowFrameMetrics metrics{
      .supported = window_ != nil,
      .client_title_bar = client_title_bar_,
      .backend = "macos",
      .mechanism = client_title_bar_
          ? "NSWindowStyleMaskFullSizeContentView, titlebarAppearsTransparent"
          : "system title bar"};
  if (window_ == nil) {
    metrics.reason = "native window closed";
    return metrics;
  }
  // Points are CGPUI logical pixels. AppKit resizes from the window edge
  // before the content view sees the event, so no band lies in the client.
  metrics.caption_height = static_cast<float>(
      NSHeight([window_ frame]) - NSHeight([window_ contentLayoutRect]));
  NSButton* close = [window_ standardWindowButton:NSWindowCloseButton];
  NSButton* zoom = [window_ standardWindowButton:NSWindowZoomButton];
  if (close != nil) {
    const NSSize size = [close bounds].size;
    metrics.caption_button_size = {static_cast<float>(size.width),
                                   static_cast<float>(size.height)};
  }
  if (client_title_bar_ && close != nil && zoom != nil && ![zoom isHidden]) {
    // The traffic lights stay native; keep their margin on both sides.
    const NSRect first = [close convertRect:[close bounds] toView:nil];
    const NSRect last = [zoom convertRect:[zoom bounds] toView:nil];
    metrics.caption_buttons_leading_reserved_width =
        static_cast<float>(NSMaxX(last) + NSMinX(first));
  }
  return metrics;
}

// Minimize, maximize and close use the base implementation. begin_move
// needs the mouse event that is being handled; macOS has no window menu,
// and resizing is driven by AppKit from the window edge.
PlatformWindowActionResult MacOSWindow::perform_window_action(
    PlatformWindowActionRequest request) {
  switch (request.action) {
    case PlatformWindowAction::begin_move: {
      NSEvent* event = [NSApp currentEvent];
      const NSEventType type = event == nil ? NSEventTypeApplicationDefined : [event type];
      if (window_ == nil || [event window] != window_ ||
          (type != NSEventTypeLeftMouseDown && type != NSEventTypeLeftMouseDragged)) {
        return {.supported = true,
                .reason = "begin_move needs a left mouse press in this window"};
      }
      [window_ performWindowDragWithEvent:event];
      return {.supported = true, .performed = true};
    }
    case PlatformWindowAction::show_window_menu:
      return {.reason = "macOS windows have no window menu"};
    case PlatformWindowAction::begin_resize:
      return {.reason = "AppKit resizes windows from their edges itself"};
    case PlatformWindowAction::minimize:
    case PlatformWindowAction::toggle_maximize:
    case PlatformWindowAction::close:
      break;
  }
  return PlatformWindow::perform_window_action(request);
}

}  // namespace cgpui
