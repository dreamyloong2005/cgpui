#pragma once

#include "cgpui/platform/platform.hpp"
#include "../platform_window_close_internal.hpp"

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

namespace cgpui {

class MacOSWindow final : public PlatformWindow {
 public:
  MacOSWindow(
      NSWindow* window,
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
  PlatformWindowChromeState apply_window_chrome(WindowChromeOptions options) override;
  void update_accessibility_tree(PlatformAccessibilityTreeUpdate update) override;

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

 private:
  __strong NSWindow* window_ = nil;
  __strong CAMetalLayer* layer_ = nil;
  __strong CGPUIMacOSWindowDelegate* delegate_ = nil;
  PlatformEventCallback callback_;
  WindowState state_;
  PlatformWindowCloseController close_controller_;
  std::function<void(MacOSWindow*)> unregister_;
  WindowChromeOptions chrome_;
  bool active_ = false;
  bool focused_ = false;
  bool minimized_ = false;
};

WindowState make_macos_window_state(NSWindow* window);

}  // namespace cgpui
