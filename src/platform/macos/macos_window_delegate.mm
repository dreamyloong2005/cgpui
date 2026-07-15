#include "macos_window_internal.hpp"

@implementation CGPUIMacOSWindowDelegate
- (instancetype)initWithWindow:(void*)window {
  self = [super init];
  if (self != nil) window_ = window;
  return self;
}
- (void)attachWindow:(void*)window { window_ = window; }
- (void)detachWindow { window_ = nullptr; }
- (BOOL)windowShouldClose:(NSWindow*)window {
  (void)window;
  if (window_ != nullptr) {
    static_cast<cgpui::MacOSWindow*>(window_)->close_requested(
        cgpui::WindowCloseRequestSource::window_manager);
  }
  return NO;
}
- (void)windowDidResize:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) static_cast<cgpui::MacOSWindow*>(window_)->resized();
}
- (void)windowDidChangeBackingProperties:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) static_cast<cgpui::MacOSWindow*>(window_)->resized();
}
- (void)windowDidMove:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) static_cast<cgpui::MacOSWindow*>(window_)->moved();
}
- (void)windowDidBecomeKey:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) static_cast<cgpui::MacOSWindow*>(window_)->focused(true);
}
- (void)windowDidResignKey:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) static_cast<cgpui::MacOSWindow*>(window_)->focused(false);
}
- (void)windowDidBecomeMain:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) static_cast<cgpui::MacOSWindow*>(window_)->activated(true);
}
- (void)windowDidResignMain:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) static_cast<cgpui::MacOSWindow*>(window_)->activated(false);
}
- (void)windowDidMiniaturize:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) static_cast<cgpui::MacOSWindow*>(window_)->minimized(true);
}
- (void)windowDidDeminiaturize:(NSNotification*)notification {
  (void)notification;
  if (window_ != nullptr) static_cast<cgpui::MacOSWindow*>(window_)->minimized(false);
}
@end
