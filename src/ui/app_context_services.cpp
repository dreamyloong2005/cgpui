#include "ui_internal.hpp"

namespace cgpui {

AppOpenedWindow AppContext::open_window(WindowOptions options) const {
  return runtime.open_window(std::move(options));
}

AppOpenedWindow AppContext::open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) const {
  return runtime.open_window(std::move(options), std::move(root_view));
}

NativeMenuInstallation AppContext::install_native_menu(
    NativeMenuModel menu) const {
  return runtime.install_native_menu(std::move(menu));
}

NativeFileDialogResult AppContext::show_native_file_dialog(
    NativeFileDialogOptions options) const {
  return runtime.show_native_file_dialog(std::move(options));
}

} // namespace cgpui
