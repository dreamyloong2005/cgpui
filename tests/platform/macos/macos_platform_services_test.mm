#include "macos_platform_services_internal.hpp"
#include "macos_window_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <cmath>
#include <variant>
#include <vector>

namespace {

bool same_size(NSSize lhs, NSSize rhs) {
  return std::fabs(lhs.width - rhs.width) < 0.01 &&
      std::fabs(lhs.height - rhs.height) < 0.01;
}

}  // namespace

int main() {
  @autoreleasepool {
    auto application = cgpui::create_platform_application();
    if (!application) return 1;
    std::vector<cgpui::PlatformEvent> events;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{.title = "CGPUI macOS Services Test",
                                .size = {240.0F, 140.0F}},
        [&](const cgpui::PlatformEvent& event) { events.push_back(event); });
    if (!window) return 2;

    const cgpui::NativeMenuModel menu{
        .items = {{
            .kind = cgpui::NativeMenuItemKind::submenu,
            .title = "File",
            .children = {
                {.title = "Open",
                 .action_name = "file.open",
                 .accelerator = cgpui::NativeMenuAccelerator{
                     .key_code = 0,
                     .modifiers = {.super = true}}},
                {.kind = cgpui::NativeMenuItemKind::separator},
                {.title = "Pinned",
                 .action_name = "file.pinned",
                 .enabled = false,
                 .checked = true},
            },
        }}};
    const auto installed = (*application)->install_native_menu(menu);
    if (!installed.supported || installed.backend != "macos" ||
        installed.menu_count != 1 || installed.item_count != 4 ||
        installed.accelerator_count != 1 ||
        installed.registered_accelerator_count != 1) {
      return 3;
    }
    NSMenu* root = [NSApp mainMenu];
    NSMenu* file = [[root itemAtIndex:0] submenu];
    NSMenuItem* open = [file itemAtIndex:0];
    NSMenuItem* pinned = [file itemAtIndex:2];
    if (![[open keyEquivalent] isEqualToString:@"a"] ||
        ([open keyEquivalentModifierMask] & NSEventModifierFlagCommand) == 0 ||
        [pinned isEnabled] || [pinned state] != NSControlStateValueOn) {
      return 4;
    }
    NSEvent* accelerator_event = [NSEvent
        keyEventWithType:NSEventTypeKeyDown
                 location:NSZeroPoint
            modifierFlags:NSEventModifierFlagCommand
                timestamp:0.0
             windowNumber:0
                  context:nil
               characters:@"a"
              charactersIgnoringModifiers:@"a"
                isARepeat:NO
                  keyCode:0];
    if (cgpui::macos_native_menu_command_source(accelerator_event) !=
            cgpui::NativeMenuCommandSource::accelerator ||
        cgpui::macos_native_menu_command_source(nil) !=
            cgpui::NativeMenuCommandSource::menu) {
      return 5;
    }
    events.clear();
    if (![NSApp sendAction:[open action] to:[open target] from:open]) return 6;
    if (events.size() != 1 ||
        !std::holds_alternative<cgpui::NativeMenuCommand>(events[0]) ||
        std::get<cgpui::NativeMenuCommand>(events[0]).action_name != "file.open") {
      return 7;
    }

    cgpui::NativeFileDialogOptions open_options{
        .kind = cgpui::NativeFileDialogKind::open_files,
        .title = "Choose Sources",
        .default_directory = "/tmp",
        .filters = {{.name = "Code", .extensions = {"cpp", "hpp"}}}};
    NSOpenPanel* open_panel = [NSOpenPanel openPanel];
    cgpui::macos_configure_open_panel(open_panel, open_options);
    if (![open_panel allowsMultipleSelection] || [open_panel canChooseDirectories] ||
        ![[open_panel title] isEqualToString:@"Choose Sources"] ||
        [open_panel allowedContentTypes].count != 2) {
      return 8;
    }
    open_options.kind = cgpui::NativeFileDialogKind::pick_directory;
    cgpui::macos_configure_open_panel(open_panel, open_options);
    if (![open_panel canChooseDirectories] || [open_panel canChooseFiles]) return 9;

    cgpui::NativeFileDialogOptions save_options{
        .kind = cgpui::NativeFileDialogKind::save_file,
        .title = "Save Document",
        .suggested_name = "notes.txt"};
    NSSavePanel* save_panel = [NSSavePanel savePanel];
    cgpui::macos_configure_save_panel(save_panel, save_options);
    if (![[save_panel nameFieldStringValue] isEqualToString:@"notes.txt"]) return 10;
    const auto cancelled_file = cgpui::macos_file_dialog_result(
        save_options, NSModalResponseCancel, @[]);
    if (!cancelled_file.supported || cancelled_file.accepted ||
        cancelled_file.backend != "macos") {
      return 11;
    }

    cgpui::NativeMessageDialogOptions message_options{
        .kind = cgpui::NativeMessageDialogKind::warning,
        .buttons = cgpui::NativeMessageDialogButtons::yes_no,
        .title = "Replace?",
        .message = "The destination exists."};
    NSAlert* alert = [[NSAlert alloc] init];
    cgpui::macos_configure_message_dialog(alert, message_options);
    if ([alert alertStyle] != NSAlertStyleWarning || [alert buttons].count != 2 ||
        ![[alert messageText] isEqualToString:@"Replace?"]) {
      return 12;
    }
    const auto yes = cgpui::macos_message_dialog_result(
        message_options, NSAlertFirstButtonReturn);
    if (!yes.supported || !yes.accepted ||
        yes.response != cgpui::NativeMessageDialogResponse::yes) {
      return 13;
    }

    if (cgpui::macos_url("https://example.com/path") == nil ||
        cgpui::macos_url("") != nil ||
        cgpui::macos_url("relative/path") != nil ||
        cgpui::macos_url(std::string_view{"a\0b", 3}) != nil) {
      return 14;
    }

    std::size_t reopen_count = 0;
    (*application)->set_reopen_callback([&] { ++reopen_count; });
    const auto reopen = (*application)->request_reopen();
    if (!reopen.supported || !reopen.requested || reopen.backend != "macos" ||
        reopen_count != 1) {
      return 15;
    }
    id<NSApplicationDelegate> delegate = [NSApp delegate];
    if ([delegate applicationShouldTerminateAfterLastWindowClosed:NSApp]) return 16;

    auto* native = dynamic_cast<cgpui::MacOSWindow*>(window->get());
    NSWindow* native_window = native->native_window();
    const NSSize content_size = [[native_window contentView] bounds].size;
    const cgpui::WindowChromeOptions chrome{
        .titlebar_visible = false,
        .decorations = false,
        .resizable = false,
        .transparent_background = true};
    const auto chrome_state = (*window)->apply_window_chrome(chrome);
    if (!chrome_state.supported || !chrome_state.decoration_control_supported ||
        !chrome_state.transparency_supported || chrome_state.backend != "macos" ||
        chrome_state.applied.titlebar_visible || chrome_state.applied.decorations ||
        chrome_state.applied.resizable ||
        !chrome_state.applied.transparent_background || [native_window isOpaque] ||
        !same_size(content_size, [[native_window contentView] bounds].size)) {
      return 17;
    }

    auto configured = (*application)->create_window(
        cgpui::WindowDescriptor{
            .title = "CGPUI macOS Configured Chrome",
            .size = {180.0F, 100.0F},
            .chrome = chrome},
        [](const cgpui::PlatformEvent&) {});
    if (!configured) return 18;
    auto* configured_native =
        dynamic_cast<cgpui::MacOSWindow*>(configured->get());
    if ([configured_native->native_window() isOpaque] ||
        [configured_native->native_window() styleMask] !=
            NSWindowStyleMaskBorderless) {
      return 19;
    }
  }
  return 0;
}
