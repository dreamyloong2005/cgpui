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

std::vector<FontFaceDescriptor> WaylandApplication::discover_font_records()
    const {
  return {
      FontFaceDescriptor{
          .font = FontDescriptor{.family = "sans-serif"},
          .postscript_name = "fontconfig:sans-serif",
          .source = FontSource::platform,
          .path = "fontconfig://sans-serif",
      },
  };
}

} // namespace cgpui
