#include "macos_application_internal.hpp"
#include "macos_platform_services_internal.hpp"
#include "macos_window_internal.hpp"

#include <string>

namespace {

NSString* native_string(std::string_view value) {
  return [[NSString alloc] initWithBytes:value.data()
                                  length:value.size()
                                encoding:NSUTF8StringEncoding];
}

NSString* key_equivalent(std::uint32_t code) {
  static constexpr char scan_codes[] = {
      'a', 's', 'd', 'f', 'h', 'g', 'z', 'x', 'c', 'v', 0, 'b',
      'q', 'w', 'e', 'r', 'y', 't'};
  if (code < sizeof(scan_codes) && scan_codes[code] != 0) {
    return [NSString stringWithFormat:@"%c", scan_codes[code]];
  }
  if ((code >= 'A' && code <= 'Z') || (code >= 'a' && code <= 'z') ||
      (code >= '0' && code <= '9')) {
    return [[NSString stringWithFormat:@"%c", static_cast<char>(code)] lowercaseString];
  }
  return @"";
}

NSEventModifierFlags modifier_flags(const cgpui::KeyboardModifiers& modifiers) {
  NSEventModifierFlags result = 0;
  if (modifiers.shift) result |= NSEventModifierFlagShift;
  if (modifiers.control) result |= NSEventModifierFlagControl;
  if (modifiers.alt) result |= NSEventModifierFlagOption;
  if (modifiers.super) result |= NSEventModifierFlagCommand;
  return result;
}

void append_items(
    NSMenu* menu,
    const std::vector<cgpui::NativeMenuItem>& items,
    CGPUIMacOSMenuTarget* target,
    std::size_t& registered,
    std::size_t& skipped) {
  for (const auto& item : items) {
    if (item.kind == cgpui::NativeMenuItemKind::separator) {
      [menu addItem:[NSMenuItem separatorItem]];
      continue;
    }
    NSString* title = native_string(item.title);
    NSMenuItem* native = [[NSMenuItem alloc]
        initWithTitle:title == nil ? @"" : title
               action:item.kind == cgpui::NativeMenuItemKind::command
                   ? @selector(performMenuCommand:) : nil
        keyEquivalent:@""];
    [native setEnabled:item.enabled];
    [native setState:item.checked ? NSControlStateValueOn : NSControlStateValueOff];
    if (item.kind == cgpui::NativeMenuItemKind::submenu) {
      NSMenu* submenu = [[NSMenu alloc] initWithTitle:title == nil ? @"" : title];
      append_items(submenu, item.children, target, registered, skipped);
      [native setSubmenu:submenu];
    } else {
      [native setTarget:target];
      [native setRepresentedObject:native_string(item.action_name) ?: @""];
      if (item.accelerator.has_value()) {
        NSString* key = item.accelerator->action == cgpui::KeyAction::pressed
            ? key_equivalent(item.accelerator->key_code) : @"";
        if (key.length == 0) {
          ++skipped;
        } else {
          [native setKeyEquivalent:key];
          [native setKeyEquivalentModifierMask:
              modifier_flags(item.accelerator->modifiers)];
          ++registered;
        }
      }
    }
    [menu addItem:native];
  }
}

}  // namespace

@implementation CGPUIMacOSMenuTarget
- (void)performMenuCommand:(NSMenuItem*)sender {
  NSString* action = [sender representedObject];
  const char* bytes = [action UTF8String];
  if (self.application != nullptr && bytes != nullptr) {
    static_cast<cgpui::MacOSApplication*>(self.application)
        ->dispatch_native_menu(
            bytes,
            cgpui::macos_native_menu_command_source([NSApp currentEvent]));
  }
}
@end

namespace cgpui {

MacOSNativeMenuBuild macos_build_native_menu(
    const NativeMenuModel& model,
    void* application) {
  CGPUIMacOSMenuTarget* target = [[CGPUIMacOSMenuTarget alloc] init];
  target.application = application;
  NSMenu* menu = [[NSMenu alloc] initWithTitle:@"CGPUI"];
  std::size_t registered = 0;
  std::size_t skipped = 0;
  append_items(menu, model.items, target, registered, skipped);
  const std::size_t accelerators = native_menu_accelerator_count(model);
  return MacOSNativeMenuBuild{
      .menu = menu,
      .target = target,
      .result = PlatformMenuInstallationResult{
          .supported = true,
          .backend = "macos",
          .menu_count = model.items.size(),
          .item_count = native_menu_item_count(model),
          .accelerator_count = accelerators,
          .registered_accelerator_count = registered,
          .skipped_accelerator_count = skipped}};
}

NativeMenuCommandSource macos_native_menu_command_source(NSEvent* event) {
  return event != nil && [event type] == NSEventTypeKeyDown
      ? NativeMenuCommandSource::accelerator
      : NativeMenuCommandSource::menu;
}

void MacOSWindow::native_menu_command(
    std::string action_name,
    NativeMenuCommandSource source) {
  callback_(NativeMenuCommand{
      .action_name = std::move(action_name),
      .source = source});
}

}  // namespace cgpui
