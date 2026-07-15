#pragma once

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <string_view>

namespace cgpui {

struct MacOSNativeMenuBuild {
  __strong NSMenu* menu = nil;
  __strong id target = nil;
  PlatformMenuInstallationResult result;
};

[[nodiscard]] MacOSNativeMenuBuild macos_build_native_menu(
    const NativeMenuModel& model,
    void* application);
[[nodiscard]] NativeMenuCommandSource macos_native_menu_command_source(
    NSEvent* event);
void macos_configure_open_panel(
    NSOpenPanel* panel,
    const NativeFileDialogOptions& options);
void macos_configure_save_panel(
    NSSavePanel* panel,
    const NativeFileDialogOptions& options);
[[nodiscard]] NativeFileDialogResult macos_file_dialog_result(
    const NativeFileDialogOptions& options,
    NSModalResponse response,
    NSArray<NSURL*>* urls);
[[nodiscard]] NativeFileDialogResult macos_show_native_file_dialog(
    const NativeFileDialogOptions& options);
void macos_configure_message_dialog(
    NSAlert* alert,
    const NativeMessageDialogOptions& options);
[[nodiscard]] NativeMessageDialogResult macos_message_dialog_result(
    const NativeMessageDialogOptions& options,
    NSModalResponse response);
[[nodiscard]] NativeMessageDialogResult macos_show_native_message_dialog(
    const NativeMessageDialogOptions& options);
[[nodiscard]] NSURL* macos_url(std::string_view value);
[[nodiscard]] PlatformOpenUrlResult macos_open_url(std::string_view value);

}  // namespace cgpui

@interface CGPUIMacOSMenuTarget : NSObject
@property(nonatomic, assign) void* application;
- (void)performMenuCommand:(NSMenuItem*)sender;
@end
