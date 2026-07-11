#include "win32_internal.hpp"

#include <utility>

namespace cgpui {

PlatformMenuInstallationResult Win32NativeMenuState::install_native_menu(
    NativeMenuModel menu) {
  auto menu_tree = Win32NativeMenuTree::build(menu);
  last_menu_installation_ = PlatformMenuInstallationResult{
      .supported = menu_tree.has_value(),
      .backend = "win32",
      .menu_count = menu.items.size(),
      .item_count = native_menu_item_count(menu),
      .accelerator_count = native_menu_accelerator_count(menu),
  };
  if (menu_tree.has_value()) {
    model_ = std::move(menu);
    menu_tree_ = std::move(*menu_tree);
  }
  return last_menu_installation_;
}

const PlatformMenuInstallationResult&
Win32NativeMenuState::last_menu_installation() const {
  return last_menu_installation_;
}

HMENU Win32NativeMenuState::native_menu() const { return menu_tree_.root(); }

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
