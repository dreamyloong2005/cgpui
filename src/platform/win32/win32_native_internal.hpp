#pragma once

#include "cgpui/platform/platform.hpp"
#include "win32_native_menu_accelerator_table_internal.hpp"
#include "win32_native_menu_command_internal.hpp"
#include "win32_native_menu_tree_internal.hpp"

namespace cgpui {

class Win32NativeMenuState {
 public:
  PlatformMenuInstallationResult install_native_menu(NativeMenuModel menu);

  [[nodiscard]] const PlatformMenuInstallationResult& last_menu_installation()
      const;
  [[nodiscard]] HMENU native_menu() const;
  [[nodiscard]] bool translate_accelerator(MSG& message) const;
  [[nodiscard]] std::shared_ptr<const Win32NativeMenuCommandMap> command_map()
      const;

 private:
  NativeMenuModel model_;
  Win32NativeMenuTree menu_tree_;
  Win32NativeMenuAcceleratorTable accelerator_table_;
  std::shared_ptr<const Win32NativeMenuCommandMap> command_map_;
  PlatformMenuInstallationResult last_menu_installation_;
};

class Win32NativeFileDialogState {
 public:
  NativeFileDialogResult show_native_file_dialog(
      NativeFileDialogOptions options);

  [[nodiscard]] const NativeFileDialogResult& last_file_dialog_result() const;

 private:
  NativeFileDialogOptions options_;
  NativeFileDialogResult last_file_dialog_result_;
};

} // namespace cgpui
