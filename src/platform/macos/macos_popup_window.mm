#include "macos_popup_window_internal.hpp"
#include "macos_input_internal.hpp"
#include "macos_window_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

// Start of an overlay of `length` on one axis: `preferred` unless it
// leaves [low, high]; clamps to `low` when the overlay is longer.
float shift_into(float preferred, float length, float low, float high, bool& shifted) {
  const float value = std::max(low, std::min(preferred, high - length));
  shifted = shifted || value != preferred;
  return value;
}

}  // namespace

MacOSPopupFrame macos_popup_frame(
    Rect anchor, Size size, WindowPopupSide side, float gap, Rect work) {
  const float left = anchor.origin.x;
  const float top = anchor.origin.y;
  const float right = left + anchor.size.width;
  const float bottom = top + anchor.size.height;
  const float work_right = work.origin.x + work.size.width;
  const float work_bottom = work.origin.y + work.size.height;
  MacOSPopupFrame frame{.bounds = {{left, top}, size}};
  const bool vertical = side == WindowPopupSide::below || side == WindowPopupSide::above;
  if (vertical) {
    const float room_below = work_bottom - bottom - gap;
    const float room_above = top - work.origin.y - gap;
    bool below = side == WindowPopupSide::below;
    if ((below && size.height > room_below && room_above > room_below) ||
        (!below && size.height > room_above && room_below > room_above)) {
      below = !below;
      frame.flipped = true;
    }
    frame.bounds.origin.y = below ? bottom + gap : top - gap - size.height;
  } else {
    const float room_right = work_right - right - gap;
    const float room_left = left - work.origin.x - gap;
    bool to_right = side == WindowPopupSide::right;
    if ((to_right && size.width > room_right && room_left > room_right) ||
        (!to_right && size.width > room_left && room_right > room_left)) {
      to_right = !to_right;
      frame.flipped = true;
    }
    frame.bounds.origin.x = to_right ? right + gap : left - gap - size.width;
  }
  frame.bounds.origin.x = shift_into(frame.bounds.origin.x, size.width, work.origin.x,
                                     work_right, frame.shifted);
  frame.bounds.origin.y = shift_into(frame.bounds.origin.y, size.height, work.origin.y,
                                     work_bottom, frame.shifted);
  return frame;
}

NSPanel* macos_make_popup_panel(const WindowDescriptor& descriptor) {
  NSPanel* panel = [[NSPanel alloc]
      initWithContentRect:NSMakeRect(0, 0, descriptor.size.width, descriptor.size.height)
                styleMask:NSWindowStyleMaskBorderless | NSWindowStyleMaskNonactivatingPanel
                  backing:NSBackingStoreBuffered
                    defer:NO];
  [panel setReleasedWhenClosed:NO];
  [panel setBecomesKeyOnlyIfNeeded:YES];
  [panel setOpaque:NO];
  [panel setBackgroundColor:[NSColor clearColor]];
  // The popup draws its own shadow (C5) inside its transparent surface.
  [panel setHasShadow:NO];
  [panel setCollectionBehavior:NSWindowCollectionBehaviorTransient |
                               NSWindowCollectionBehaviorIgnoresCycle |
                               NSWindowCollectionBehaviorFullScreenAuxiliary];
  return panel;
}

void macos_attach_popup_panel(NSPanel* panel, NSWindow* parent) {
  [parent addChildWindow:panel ordered:NSWindowAbove];
  // addChildWindow: gives the child its parent's level; set these after.
  [panel setLevel:NSPopUpMenuWindowLevel];
  [panel setHidesOnDeactivate:YES];
}

namespace {

// CGPUI's flipped content view of a window (the content view, or its only
// subview while a backdrop material wraps it).
NSView* cgpui_view(NSWindow* window) {
  NSView* content = [window contentView];
  if ([content isKindOfClass:[CGPUIMacOSContentView class]]) return content;
  for (NSView* subview in [content subviews]) {
    if ([subview isKindOfClass:[CGPUIMacOSContentView class]]) return subview;
  }
  return content;
}

// AppKit screen space is y-up from the primary screen's bottom left; the
// placement math is y-down from its top left.
CGFloat primary_top() {
  NSScreen* primary = [[NSScreen screens] firstObject];
  return primary == nil ? 0.0 : NSMaxY([primary frame]);
}

Rect y_down(NSRect rect) {
  return Rect{{static_cast<float>(rect.origin.x),
               static_cast<float>(primary_top() - NSMaxY(rect))},
              {static_cast<float>(rect.size.width), static_cast<float>(rect.size.height)}};
}

NSRect y_up(Rect rect) {
  return NSMakeRect(rect.origin.x, primary_top() - rect.origin.y - rect.size.height,
                    rect.size.width, rect.size.height);
}

NSScreen* screen_for(NSRect rect, NSWindow* fallback) {
  const NSPoint center = NSMakePoint(NSMidX(rect), NSMidY(rect));
  for (NSScreen* screen in [NSScreen screens]) {
    if (NSPointInRect(center, [screen frame])) return screen;
  }
  return [fallback screen] != nil ? [fallback screen] : [NSScreen mainScreen];
}

}  // namespace

PlatformPopupPlacementResult MacOSWindow::place_popup(WindowPopupPlacement placement) {
  PlatformPopupPlacementResult result{.supported = popup_};
  NSWindow* parent = popup_parent_;
  if (!popup_ || window_ == nil || parent == nil) {
    result.reason = popup_ ? "popup parent window closed" : "not a popup window";
    return result;
  }
  NSView* view = cgpui_view(parent);
  const NSRect anchor_window = [view convertRect:NSMakeRect(placement.anchor.origin.x,
                                                            placement.anchor.origin.y,
                                                            placement.anchor.size.width,
                                                            placement.anchor.size.height)
                                          toView:nil];
  const NSRect anchor_screen = [parent convertRectToScreen:anchor_window];
  NSScreen* screen = screen_for(anchor_screen, parent);
  const NSSize size = [window_ frame].size;
  const MacOSPopupFrame frame = macos_popup_frame(
      y_down(anchor_screen),
      Size{static_cast<float>(size.width), static_cast<float>(size.height)},
      placement.side, placement.gap, y_down([screen visibleFrame]));
  const NSRect screen_frame = y_up(frame.bounds);
  // Moving sends windowDidMove:, and the runtime may lay out again in
  // response; an unchanged origin must not move the window.
  if (!NSEqualPoints([window_ frame].origin, screen_frame.origin)) {
    [window_ setFrameOrigin:screen_frame.origin];
  }
  const NSRect in_parent =
      [view convertRect:[parent convertRectFromScreen:screen_frame] fromView:nil];
  result.placed = true;
  result.flipped = frame.flipped;
  result.shifted = frame.shifted;
  result.bounds = Rect{{static_cast<float>(in_parent.origin.x),
                        static_cast<float>(in_parent.origin.y)},
                       {static_cast<float>(in_parent.size.width),
                        static_cast<float>(in_parent.size.height)}};
  return result;
}

void MacOSWindow::attach_popup_parent(NSWindow* parent) {
  popup_ = true;
  popup_parent_ = parent;
}

}  // namespace cgpui
