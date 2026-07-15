#pragma once

#include "cgpui/platform/platform.hpp"
#include "../platform_window_close_internal.hpp"
#include "macos_text_services_internal.hpp"

#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include <functional>
#include <memory>

@interface CGPUIMacOSWindowDelegate : NSObject <NSWindowDelegate> {
 @private
  void* window_;
}
- (instancetype)initWithWindow:(void*)window;
- (void)attachWindow:(void*)window;
- (void)detachWindow;
@end

@class CGPUIMacOSContentView;

namespace cgpui {

class MacOSWindow final : public PlatformWindow {
 public:
  MacOSWindow(
      NSWindow* window,
      CGPUIMacOSContentView* content_view,
      CAMetalLayer* layer,
      CGPUIMacOSWindowDelegate* delegate,
      PlatformEventCallback callback,
      WindowState state,
      std::function<void(MacOSWindow*)> unregister);
  ~MacOSWindow() override;

  [[nodiscard]] NativeSurfaceHandle native_surface() const override;
  [[nodiscard]] WindowState state() const override;
  [[nodiscard]] PlatformWindowLifecycleState lifecycle_state() const override;
  bool request_display_state(PlatformWindowDisplayState display_state) override;
  [[nodiscard]] PlatformWindowPositionState position_state() const override;
  bool request_position(Point position) override;
  [[nodiscard]] PlatformWindowCloseState close_request_state() const override;
  bool resolve_close_request(PlatformWindowCloseResolution resolution) override;
  void request_redraw() override;
  void request_close() override;
  void set_title(std::string_view title) override;
  void set_cursor(CursorShape cursor_shape) override;
  [[nodiscard]] PlatformPointerCaptureState pointer_capture_state() const override;
  void set_pointer_capture(bool captured) override;
  void set_ime_text_input_placement(
      std::optional<ImeTextInputPlacement> placement) override;

  void text_insert(NSString* string, NSRange replacement_range);
  void text_set_marked(
      NSString* string,
      NSRange selected_range,
      NSRange replacement_range);
  void text_unmark();
  void text_delete_backward();
  [[nodiscard]] BOOL text_has_marked() const;
  [[nodiscard]] NSRange text_marked_range() const;
  [[nodiscard]] NSRange text_selected_range() const;
  [[nodiscard]] NSAttributedString* text_substring(
      NSRange proposed_range,
      NSRange* actual_range) const;
  [[nodiscard]] NSRect text_first_rect(
      NSRange character_range,
      NSRange* actual_range) const;
  [[nodiscard]] NSUInteger text_character_index(NSPoint point) const;
  [[nodiscard]] MacOSTextServicesDiagnostics text_services_diagnostics() const {
    return text_input_state_.diagnostics;
  }

  [[nodiscard]] NSWindow* native_window() const { return window_; }
  void detach_registry() { unregister_ = {}; }
  void refresh_state();
  void close_requested(WindowCloseRequestSource source);
  void resized();
  void moved();
  void activated(bool active);
  void focused(bool focused);
  void minimized(bool minimized);
  void wakeup_requested();
  void pointer_moved(NSEvent* event);
  void pointer_exited(NSEvent* event);
  void pointer_button(NSEvent* event, bool pressed);
  void pointer_scrolled(NSEvent* event);
  void keyboard_key(NSEvent* event, bool pressed);
  void drag_entered(
      Point position,
      DragDropPayload payload,
      DragDropAction action);
  void drag_updated(Point position, DragDropAction action);
  void drag_dropped(
      Point position,
      DragDropPayload payload,
      DragDropAction action);
  void drag_exited(Point position);
  [[nodiscard]] Point event_position(NSEvent* event) const;
  void release_pointer_capture(bool notify);
  [[nodiscard]] CursorShape cursor_shape() const { return cursor_shape_; }

 private:
  __strong NSWindow* window_ = nil;
  __strong CGPUIMacOSContentView* content_view_ = nil;
  __strong CAMetalLayer* layer_ = nil;
  __strong CGPUIMacOSWindowDelegate* delegate_ = nil;
  __strong id pointer_monitor_ = nil;
  PlatformEventCallback callback_;
  WindowState state_;
  PlatformWindowCloseController close_controller_;
  std::function<void(MacOSWindow*)> unregister_;
  bool active_ = false;
  bool focused_ = false;
  bool minimized_ = false;
  bool pointer_captured_ = false;
  CursorShape cursor_shape_ = CursorShape::default_arrow;
  DragDropPayload drag_payload_;
  Point drag_position_;
  bool drag_active_ = false;
  MacOSTextInputState text_input_state_;
};

WindowState make_macos_window_state(NSWindow* window);

}  // namespace cgpui
