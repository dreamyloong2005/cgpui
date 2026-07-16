#include "x11_application_internal.hpp"

namespace cgpui {

PlatformMenuInstallationResult X11Application::install_native_menu(
    NativeMenuModel menu) {
  const std::size_t accelerator_count = native_menu_accelerator_count(menu);
  return PlatformMenuInstallationResult{
      .supported = false,
      .backend = "x11",
      .menu_count = menu.items.size(),
      .item_count = native_menu_item_count(menu),
      .accelerator_count = accelerator_count,
      .registered_accelerator_count = 0,
      .skipped_accelerator_count = accelerator_count,
  };
}

NativeFileDialogResult X11Application::show_native_file_dialog(
    NativeFileDialogOptions options) {
  return NativeFileDialogResult{
      .supported = false,
      .accepted = false,
      .backend = "x11",
      .kind = options.kind,
      .error_message = "native file dialog unsupported by x11",
      .filter_count = options.filters.size(),
  };
}

NativeMessageDialogResult X11Application::show_native_message_dialog(
    NativeMessageDialogOptions) {
  return NativeMessageDialogResult{
      .supported = false,
      .backend = "x11",
      .error_message = "native message dialog unsupported by x11",
  };
}

PlatformOpenUrlResult X11Application::open_url(std::string) {
  return PlatformOpenUrlResult{
      .supported = false,
      .backend = "x11",
      .error_message = "open URL unsupported by x11",
  };
}

}  // namespace cgpui
