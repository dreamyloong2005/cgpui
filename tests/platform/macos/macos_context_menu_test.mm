#include "macos_window_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <variant>
#include <vector>

namespace {

// While the next menu tracks (AppKit's modal loop), check it and then
// choose item `index` (or none for -1) and close it.
id close_next_menu(NSInteger index, bool& disabled_seen) {
  bool* seen = &disabled_seen;
  return [[NSNotificationCenter defaultCenter]
      addObserverForName:NSMenuDidBeginTrackingNotification
                  object:nil
                   queue:nil
              usingBlock:^(NSNotification* note) {
                NSMenu* menu = note.object;
                CFRunLoopPerformBlock(CFRunLoopGetMain(), kCFRunLoopCommonModes, ^{
                  for (NSMenuItem* item in [menu itemArray]) {
                    if ([[item title] isEqualToString:@"Delete"] && ![item isEnabled]) {
                      *seen = true;
                    }
                  }
                  if (index >= 0) [menu performActionForItemAtIndex:index];
                  [menu cancelTracking];
                });
                CFRunLoopWakeUp(CFRunLoopGetMain());
              }];
}

const cgpui::NativeMenuModel menu{.items = {
    {.title = "New File...", .action_name = "file.new"},
    {.kind = cgpui::NativeMenuItemKind::separator},
    {.title = "Rename", .action_name = "edit.rename"},
    {.title = "Delete", .action_name = "edit.delete", .enabled = false},
    {.kind = cgpui::NativeMenuItemKind::submenu, .title = "Open With",
     .children = {{.title = "Text Editor", .action_name = "open.text"}}}}};

}  // namespace

int main() {
  @autoreleasepool {
    auto application = cgpui::create_platform_application();
    if (!application) return 1;
    std::vector<cgpui::PlatformEvent> events;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{.title = "CGPUI macOS Context Menu Test",
                                .size = {320.0F, 240.0F}},
        [&](const cgpui::PlatformEvent& event) { events.push_back(event); });
    if (!window) return 2;

    // Choosing "Rename" returns it and sends it as a context-menu command.
    bool disabled_seen = false;
    events.clear();
    id observer = close_next_menu(2, disabled_seen);
    const auto chosen = (*window)->show_context_menu(menu, {40.0F, 60.0F});
    [[NSNotificationCenter defaultCenter] removeObserver:observer];
    // The command event follows on the next main-queue turn.
    for (const auto& event : events) {
      if (std::holds_alternative<cgpui::NativeMenuCommand>(event)) return 7;
    }
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
    const cgpui::NativeMenuCommand* command = nullptr;
    for (const auto& event : events) {
      if (const auto* value = std::get_if<cgpui::NativeMenuCommand>(&event)) command = value;
    }
    if (!chosen.supported || !chosen.shown || chosen.backend != "macos" ||
        chosen.action_name != "edit.rename" || !disabled_seen || command == nullptr ||
        command->action_name != "edit.rename" ||
        command->source != cgpui::NativeMenuCommandSource::context_menu) {
      return 3;
    }

    // Closing without a choice sends nothing.
    events.clear();
    observer = close_next_menu(-1, disabled_seen);
    const auto dismissed = (*window)->show_context_menu(menu, {40.0F, 60.0F});
    [[NSNotificationCenter defaultCenter] removeObserver:observer];
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
    if (!dismissed.shown || dismissed.action_name.has_value()) return 4;
    for (const auto& event : events) {
      if (std::holds_alternative<cgpui::NativeMenuCommand>(event)) return 5;
    }

    const auto empty = (*window)->show_context_menu({}, {0.0F, 0.0F});
    return empty.supported && !empty.shown && !empty.reason.empty() ? 0 : 6;
  }
}
