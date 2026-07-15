#include "macos_input_internal.hpp"
#include "macos_drag_internal.hpp"
#include "macos_window_internal.hpp"

namespace {

// NSTextInputClient selectors stay at the Cocoa view boundary.

cgpui::MacOSWindow* adapter(void* value) {
  return static_cast<cgpui::MacOSWindow*>(value);
}

}  // namespace

@implementation CGPUIMacOSContentView
- (instancetype)initWithFrame:(NSRect)frame windowAdapter:(void*)windowAdapter {
  self = [super initWithFrame:frame];
  if (self != nil) {
    window_adapter_ = windowAdapter;
    [self registerForDraggedTypes:@[
      NSPasteboardTypeString,
      NSPasteboardTypeFileURL,
      NSPasteboardTypePNG,
      NSPasteboardTypeTIFF,
    ]];
  }
  return self;
}
- (void)attachWindowAdapter:(void*)windowAdapter {
  window_adapter_ = windowAdapter;
}
- (void)detachWindowAdapter {
  window_adapter_ = nullptr;
}
- (BOOL)isFlipped {
  return YES;
}
- (BOOL)acceptsFirstResponder {
  return YES;
}
- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
  if (window_adapter_ == nullptr) return;
  NSString* value = [string isKindOfClass:[NSAttributedString class]]
      ? [string string] : string;
  adapter(window_adapter_)->text_insert(value, replacementRange);
}
- (void)setMarkedText:(id)string
        selectedRange:(NSRange)selectedRange
      replacementRange:(NSRange)replacementRange {
  if (window_adapter_ == nullptr) return;
  NSString* value = [string isKindOfClass:[NSAttributedString class]]
      ? [string string] : string;
  adapter(window_adapter_)->text_set_marked(value, selectedRange, replacementRange);
}
- (void)unmarkText {
  if (window_adapter_ != nullptr) adapter(window_adapter_)->text_unmark();
}
- (BOOL)hasMarkedText {
  return window_adapter_ != nullptr && adapter(window_adapter_)->text_has_marked();
}
- (NSRange)markedRange {
  return window_adapter_ == nullptr ? NSMakeRange(NSNotFound, 0)
                                    : adapter(window_adapter_)->text_marked_range();
}
- (NSRange)selectedRange {
  return window_adapter_ == nullptr ? NSMakeRange(0, 0)
                                    : adapter(window_adapter_)->text_selected_range();
}
- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText {
  return @[];
}
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRange*)actualRange {
  return window_adapter_ == nullptr
      ? [[NSAttributedString alloc] initWithString:@""]
      : adapter(window_adapter_)->text_substring(range, actualRange);
}
- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRange*)actualRange {
  return window_adapter_ == nullptr ? NSZeroRect
                                    : adapter(window_adapter_)->text_first_rect(range, actualRange);
}
- (NSUInteger)characterIndexForPoint:(NSPoint)point {
  return window_adapter_ == nullptr ? 0 : adapter(window_adapter_)->text_character_index(point);
}
- (void)doCommandBySelector:(SEL)selector {
  if (window_adapter_ != nullptr && selector == @selector(deleteBackward:)) {
    adapter(window_adapter_)->text_delete_backward();
  }
}
- (void)updateTrackingAreas {
  if (tracking_area_ != nil) [self removeTrackingArea:tracking_area_];
  tracking_area_ = [[NSTrackingArea alloc]
      initWithRect:NSZeroRect
           options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved |
                   NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect
             owner:self
          userInfo:nil];
  [self addTrackingArea:tracking_area_];
  [super updateTrackingAreas];
}
- (void)mouseMoved:(NSEvent*)event {
  if (window_adapter_ != nullptr) adapter(window_adapter_)->pointer_moved(event);
}
- (void)mouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}
- (void)rightMouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}
- (void)otherMouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}
- (void)mouseExited:(NSEvent*)event {
  if (window_adapter_ != nullptr) adapter(window_adapter_)->pointer_exited(event);
}
- (void)mouseDown:(NSEvent*)event {
  if (window_adapter_ != nullptr) adapter(window_adapter_)->pointer_button(event, true);
}
- (void)mouseUp:(NSEvent*)event {
  if (window_adapter_ != nullptr) adapter(window_adapter_)->pointer_button(event, false);
}
- (void)rightMouseDown:(NSEvent*)event {
  [self mouseDown:event];
}
- (void)rightMouseUp:(NSEvent*)event {
  [self mouseUp:event];
}
- (void)otherMouseDown:(NSEvent*)event {
  [self mouseDown:event];
}
- (void)otherMouseUp:(NSEvent*)event {
  [self mouseUp:event];
}
- (void)scrollWheel:(NSEvent*)event {
  if (window_adapter_ != nullptr) adapter(window_adapter_)->pointer_scrolled(event);
}
- (void)keyDown:(NSEvent*)event {
  if (window_adapter_ != nullptr) {
    adapter(window_adapter_)->keyboard_key(event, true);
    [self interpretKeyEvents:@[event]];
  }
}
- (void)keyUp:(NSEvent*)event {
  if (window_adapter_ != nullptr) adapter(window_adapter_)->keyboard_key(event, false);
}
- (void)flagsChanged:(NSEvent*)event {
  if (window_adapter_ != nullptr) {
    adapter(window_adapter_)->keyboard_key(
        event,
        cgpui::macos_modifier_key_pressed(
            [event keyCode], [event modifierFlags]));
  }
}
- (void)resetCursorRects {
  [super resetCursorRects];
  if (window_adapter_ != nullptr) {
    [self addCursorRect:[self bounds]
                 cursor:cgpui::macos_cursor_for_shape(
                            adapter(window_adapter_)->cursor_shape())];
  }
}
- (void)dispatchCapturedMouseEvent:(NSEvent*)event {
  switch ([event type]) {
    case NSEventTypeLeftMouseDown:
    case NSEventTypeRightMouseDown:
    case NSEventTypeOtherMouseDown: [self mouseDown:event]; break;
    case NSEventTypeLeftMouseUp:
    case NSEventTypeRightMouseUp:
    case NSEventTypeOtherMouseUp: [self mouseUp:event]; break;
    default: [self mouseMoved:event]; break;
  }
}
@end
