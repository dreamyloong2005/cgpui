#include "macos_input_internal.hpp"
#include "macos_platform_services_internal.hpp"
#include "macos_window_internal.hpp"

#include <dispatch/dispatch.h>

#include <memory>
#include <optional>
#include <string>

// Records the chosen command; the window dispatches it after the menu
// closes, so the app never runs inside AppKit's menu tracking loop.
@interface CGPUIMacOSContextMenuTarget : NSObject
@property(nonatomic, copy) NSString* chosen;
- (void)performMenuCommand:(NSMenuItem*)sender;
@end

@implementation CGPUIMacOSContextMenuTarget
- (void)performMenuCommand:(NSMenuItem*)sender {
  id action = [sender representedObject];
  self.chosen = [action isKindOfClass:[NSString class]] ? action : @"";
}
@end

namespace cgpui {

// NSMenu popUpMenuPositioningItem:atLocation:inView: tracks the menu in a
// modal loop: it returns when the menu closes, and the default-mode run
// loop (timers, animations, rendering) waits until then.
PlatformContextMenuResult MacOSWindow::show_context_menu(
    const NativeMenuModel& menu, Point position) {
  if (![NSThread isMainThread]) {
    __block std::optional<PlatformContextMenuResult> result;
    dispatch_sync(dispatch_get_main_queue(), ^{
      result = show_context_menu(menu, position);
    });
    return std::move(*result);
  }
  PlatformContextMenuResult result{.supported = true, .backend = "macos"};
  if (window_ == nil || content_view_ == nil) {
    result.reason = "native window closed";
    return result;
  }
  if (menu.items.empty()) {
    result.reason = "the menu has no items";
    return result;
  }
  CGPUIMacOSContextMenuTarget* target = [[CGPUIMacOSContextMenuTarget alloc] init];
  NSMenu* native = macos_build_menu(menu, target);
  // The content view is flipped, so logical client pixels are view points.
  [native popUpMenuPositioningItem:nil
                        atLocation:NSMakePoint(position.x, position.y)
                            inView:content_view_];
  result.shown = true;
  if (target.chosen != nil) {
    result.action_name = std::string([target.chosen UTF8String]);
    // The caller is usually still handling the click that opened the menu;
    // the command event follows on the next main-queue turn instead of
    // re-entering it.
    std::weak_ptr<MacOSWindow*> alive = lifetime_;
    std::string action = *result.action_name;
    dispatch_async(dispatch_get_main_queue(), ^{
      if (const auto window = alive.lock()) {
        (*window)->native_menu_command(action, NativeMenuCommandSource::context_menu);
      }
    });
  }
  return result;
}

}  // namespace cgpui
