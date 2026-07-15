#include "macos_application_internal.hpp"
#include "macos_platform_services_internal.hpp"
#include "macos_window_internal.hpp"

#include <dispatch/dispatch.h>

#include <functional>
#include <optional>

namespace cgpui {
namespace {

template <typename Result>
Result on_main_thread(std::function<Result()> operation) {
  if ([NSThread isMainThread]) return operation();
  __block std::optional<Result> result;
  dispatch_sync(dispatch_get_main_queue(), ^{ result = operation(); });
  return std::move(*result);
}

}  // namespace

PlatformMenuInstallationResult MacOSApplication::install_native_menu(
    NativeMenuModel menu) {
  return on_main_thread<PlatformMenuInstallationResult>(
      [this, menu = std::move(menu)] {
        if ([menu_target_ isKindOfClass:[CGPUIMacOSMenuTarget class]]) {
          ((CGPUIMacOSMenuTarget*)menu_target_).application = nullptr;
        }
        auto build = macos_build_native_menu(menu, this);
        menu_target_ = build.target;
        main_menu_ = build.menu;
        [NSApp setMainMenu:main_menu_];
        return build.result;
      });
}

NativeFileDialogResult MacOSApplication::show_native_file_dialog(
    NativeFileDialogOptions options) {
  return on_main_thread<NativeFileDialogResult>(
      [options = std::move(options)] {
        return macos_show_native_file_dialog(options);
      });
}

NativeMessageDialogResult MacOSApplication::show_native_message_dialog(
    NativeMessageDialogOptions options) {
  return on_main_thread<NativeMessageDialogResult>(
      [options = std::move(options)] {
        return macos_show_native_message_dialog(options);
      });
}

PlatformOpenUrlResult MacOSApplication::open_url(std::string url) {
  return on_main_thread<PlatformOpenUrlResult>(
      [url = std::move(url)] { return macos_open_url(url); });
}

void MacOSApplication::dispatch_native_menu(
    std::string action_name,
    NativeMenuCommandSource source) {
  NSWindow* key = [NSApp keyWindow];
  MacOSWindow* target = nullptr;
  for (MacOSWindow* window : windows_) {
    if (window != nullptr && window->native_window() == key) {
      target = window;
      break;
    }
  }
  if (target == nullptr && !windows_.empty()) target = windows_.front();
  if (target != nullptr) {
    target->native_menu_command(std::move(action_name), source);
  }
}

}  // namespace cgpui
