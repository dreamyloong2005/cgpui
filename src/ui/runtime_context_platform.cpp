#include "ui_internal.hpp"

#include <utility>

namespace cgpui {

NativeMenuInstallation WindowRuntimeContext::install_native_menu(
    NativeMenuModel menu) const {
  return runtime.install_native_menu(std::move(menu));
}

Result<NativeMenuInstallation> WindowRuntimeContext::try_install_native_menu(
    NativeMenuModel menu) const {
  return runtime.try_install_native_menu(std::move(menu));
}

const NativeMenuInstallation& WindowRuntimeContext::native_menu_installation()
    const {
  return runtime.native_menu_installation();
}

NativeFileDialogResult WindowRuntimeContext::show_native_file_dialog(
    NativeFileDialogOptions options) const {
  return runtime.show_native_file_dialog(std::move(options));
}

Result<NativeFileDialogResult>
WindowRuntimeContext::try_show_native_file_dialog(
    NativeFileDialogOptions options) const {
  return runtime.try_show_native_file_dialog(std::move(options));
}

const NativeFileDialogResult& WindowRuntimeContext::native_file_dialog_result()
    const {
  return runtime.native_file_dialog_result();
}

void WindowRuntimeContext::set_window_theme(Theme theme) const {
  runtime.set_window_theme(window_runtime_id, std::move(theme));
}

bool WindowRuntimeContext::clear_window_theme() const {
  return runtime.clear_window_theme(window_runtime_id);
}

const Theme& WindowRuntimeContext::app_theme() const {
  return runtime.app_theme();
}

const Theme* WindowRuntimeContext::window_theme() const {
  return runtime.window_theme(window_runtime_id);
}

std::optional<Color> WindowRuntimeContext::theme_color(
    const ThemeTokenId& id) const {
  return runtime.theme_color(window_runtime_id, id);
}

std::optional<float> WindowRuntimeContext::theme_spacing(
    const ThemeTokenId& id) const {
  return runtime.theme_spacing(window_runtime_id, id);
}

} // namespace cgpui
