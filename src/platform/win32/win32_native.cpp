#include "win32_internal.hpp"

#include <utility>

namespace cgpui {

PlatformMenuInstallationResult Win32NativeMenuState::install_native_menu(
    NativeMenuModel menu) {
  model_ = std::move(menu);
  last_menu_installation_ = PlatformMenuInstallationResult{
      .supported = false,
      .backend = "win32",
      .menu_count = model_.items.size(),
      .item_count = native_menu_item_count(model_),
      .accelerator_count = native_menu_accelerator_count(model_),
  };
  return last_menu_installation_;
}

const PlatformMenuInstallationResult&
Win32NativeMenuState::last_menu_installation() const {
  return last_menu_installation_;
}

NativeFileDialogResult Win32NativeFileDialogState::show_native_file_dialog(
    NativeFileDialogOptions options) {
  options_ = std::move(options);
  last_file_dialog_result_ = NativeFileDialogResult{
      .supported = false,
      .accepted = false,
      .backend = "win32",
      .kind = options_.kind,
      .filter_count = options_.filters.size(),
  };
  return last_file_dialog_result_;
}

const NativeFileDialogResult&
Win32NativeFileDialogState::last_file_dialog_result() const {
  return last_file_dialog_result_;
}

} // namespace cgpui
