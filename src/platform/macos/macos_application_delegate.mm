#include "macos_application_internal.hpp"
#include "macos_platform_services_internal.hpp"
#include "macos_window_internal.hpp"

@interface CGPUIMacOSApplicationDelegate : NSObject <NSApplicationDelegate>
@property(nonatomic, assign) void* application;
@end

@implementation CGPUIMacOSApplicationDelegate
- (void)applicationDidFinishLaunching:(NSNotification*)notification {
  (void)notification;
}
- (BOOL)applicationShouldHandleReopen:(NSApplication*)application
                     hasVisibleWindows:(BOOL)hasVisibleWindows {
  (void)application;
  (void)hasVisibleWindows;
  if (self.application != nullptr) {
    static_cast<cgpui::MacOSApplication*>(self.application)->reopened();
  }
  return YES;
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication*)application {
  (void)application;
  return NO;
}
@end

@implementation CGPUIMacOSWakeupTarget
- (void)dispatchWakeup {
  if (self.application != nullptr) {
    static_cast<cgpui::MacOSApplication*>(self.application)->dispatch_wakeup();
  }
}
- (void)dispatchDelayedWakeup:(NSTimer*)timer {
  (void)timer;
  self.timer = nil;
  [self dispatchWakeup];
}
- (void)scheduleDelayedWakeup:(NSNumber*)delayMilliseconds {
  [self cancelDelayedWakeup];
  const NSTimeInterval delay = [delayMilliseconds unsignedLongLongValue] / 1000.0;
  self.timer = [NSTimer scheduledTimerWithTimeInterval:delay
                                                target:self
                                              selector:@selector(dispatchDelayedWakeup:)
                                              userInfo:nil
                                               repeats:NO];
}
- (void)cancelDelayedWakeup {
  [self.timer invalidate];
  self.timer = nil;
}
@end

namespace cgpui {

MacOSApplication::MacOSApplication() {
  NSApplication* application = [NSApplication sharedApplication];
  [application setActivationPolicy:NSApplicationActivationPolicyRegular];
  delegate_ = [[CGPUIMacOSApplicationDelegate alloc] init];
  delegate_.application = this;
  [application setDelegate:delegate_];
  wakeup_target_ = [[CGPUIMacOSWakeupTarget alloc] init];
  wakeup_target_.application = this;
}

MacOSApplication::~MacOSApplication() {
  macos_cancel_wakeup(wakeup_target_);
  if ([NSApp delegate] == delegate_) [NSApp setDelegate:nil];
  delegate_.application = nullptr;
  wakeup_target_.application = nullptr;
  if ([menu_target_ isKindOfClass:[CGPUIMacOSMenuTarget class]]) {
    ((CGPUIMacOSMenuTarget*)menu_target_).application = nullptr;
  }
  if ([NSApp mainMenu] == main_menu_) [NSApp setMainMenu:nil];
  for (MacOSWindow* window : windows_) {
    if (window != nullptr) window->detach_registry();
  }
  windows_.clear();
}

Result<std::unique_ptr<PlatformWindow>> MacOSApplication::create_window(
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback) {
  auto window = create_macos_window(
      descriptor,
      nil,
      std::move(callback),
      [this](MacOSWindow* value) { unregister_window(value); });
  if (!window) return std::unexpected(window.error());
  register_window(window->get());
  return std::unique_ptr<PlatformWindow>(std::move(*window));
}

Result<std::unique_ptr<PlatformWindow>> MacOSApplication::create_child_window(
    const WindowDescriptor& descriptor,
    PlatformWindow& parent,
    PlatformEventCallback callback) {
  auto* macos_parent = dynamic_cast<MacOSWindow*>(&parent);
  if (macos_parent == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "Cocoa child window requires a Cocoa parent"});
  }
  auto window = create_macos_window(
      descriptor,
      macos_parent->native_window(),
      std::move(callback),
      [this](MacOSWindow* value) { unregister_window(value); });
  if (!window) return std::unexpected(window.error());
  register_window(window->get());
  return std::unique_ptr<PlatformWindow>(std::move(*window));
}

void MacOSApplication::register_window(MacOSWindow* window) {
  if (window != nullptr) windows_.push_back(window);
}

void MacOSApplication::unregister_window(MacOSWindow* window) {
  windows_.erase(
      std::remove(windows_.begin(), windows_.end(), window), windows_.end());
}

void MacOSApplication::reopened() { (void)dispatch_reopen("macos"); }

PlatformReopenResult MacOSApplication::request_reopen() {
  return dispatch_reopen("macos");
}

}  // namespace cgpui
