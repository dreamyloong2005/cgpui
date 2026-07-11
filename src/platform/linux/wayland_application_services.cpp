#include "wayland_application_internal.hpp"

namespace cgpui {

PlatformMenuInstallationResult WaylandApplication::install_native_menu(
    NativeMenuModel menu) {
  last_menu_installation_ =
      wayland_install_native_menu(*native_menu_state_, std::move(menu));
  return last_menu_installation_;
}

NativeFileDialogResult WaylandApplication::show_native_file_dialog(
    NativeFileDialogOptions options) {
  last_file_dialog_result_ =
      wayland_show_native_file_dialog(
          *native_file_dialog_state_,
          std::move(options));
  return last_file_dialog_result_;
}

NativeMessageDialogResult WaylandApplication::show_native_message_dialog(
    NativeMessageDialogOptions) {
  return NativeMessageDialogResult{
      .supported = false,
      .backend = "wayland",
      .error_message = "native message dialog unsupported by wayland",
  };
}

} // namespace cgpui
