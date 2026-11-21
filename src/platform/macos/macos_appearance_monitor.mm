#include "macos_system_appearance_internal.hpp"

#import <AppKit/AppKit.h>

#include <utility>

static void* CGPUIEffectiveAppearanceContext = &CGPUIEffectiveAppearanceContext;

@interface CGPUIMacOSAppearanceObserver : NSObject
@property(nonatomic, assign) cgpui::MacOSAppearanceMonitor* monitor;
- (void)start;
- (void)stop;
@end

@implementation CGPUIMacOSAppearanceObserver {
  BOOL started_;
}
- (void)start {
  if (started_) return;
  started_ = YES;
  [NSApp addObserver:self
          forKeyPath:@"effectiveAppearance"
             options:NSKeyValueObservingOptionNew
             context:CGPUIEffectiveAppearanceContext];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
         selector:@selector(systemColorsChanged:)
             name:NSSystemColorsDidChangeNotification
           object:nil];
  [[[NSWorkspace sharedWorkspace] notificationCenter]
      addObserver:self
         selector:@selector(accessibilityDisplayOptionsChanged:)
             name:NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification
           object:nil];
}
- (void)stop {
  if (!started_) return;
  started_ = NO;
  [NSApp removeObserver:self
             forKeyPath:@"effectiveAppearance"
                context:CGPUIEffectiveAppearanceContext];
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
}
- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSKeyValueChangeKey, id>*)change
                       context:(void*)context {
  if (context != CGPUIEffectiveAppearanceContext) {
    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    return;
  }
  if (self.monitor != nullptr) {
    self.monitor->handle(cgpui::SystemAppearanceChangeSource::color_set);
  }
}
- (void)systemColorsChanged:(NSNotification*)notification {
  (void)notification;
  if (self.monitor != nullptr) {
    self.monitor->handle(cgpui::SystemAppearanceChangeSource::colorization);
  }
}
- (void)accessibilityDisplayOptionsChanged:(NSNotification*)notification {
  (void)notification;
  if (self.monitor != nullptr) self.monitor->handle_accessibility_change();
}
@end

namespace cgpui {

MacOSAppearanceMonitor::~MacOSAppearanceMonitor() {
  observer_.monitor = nullptr;
  [observer_ stop];
}

void MacOSAppearanceMonitor::set_callback(SystemAppearanceCallback callback) {
  callback_ = std::move(callback);
  if (observer_ != nil) return;
  last_appearance_ = macos_read_system_appearance();
  observer_ = [[CGPUIMacOSAppearanceObserver alloc] init];
  observer_.monitor = this;
  [observer_ start];
}

void MacOSAppearanceMonitor::handle(SystemAppearanceChangeSource source) {
  SystemAppearanceChange change{
      .source = source, .appearance = macos_read_system_appearance()};
  change.changed = !(change.appearance == last_appearance_);
  last_appearance_ = change.appearance;
  if (!callback_) return;
  const SystemAppearanceCallback callback = callback_;
  callback(change);
}

void MacOSAppearanceMonitor::handle_accessibility_change() {
  handle(macos_accessibility_change_source(
      last_appearance_, macos_read_system_appearance()));
}

} // namespace cgpui
