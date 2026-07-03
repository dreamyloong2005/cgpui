#pragma once

#include "cgpui/platform/platform.hpp"

namespace cgpui {

class Win32NativeMenuState {
 public:
  PlatformMenuInstallationResult install_native_menu(NativeMenuModel menu);

  [[nodiscard]] const PlatformMenuInstallationResult& last_menu_installation()
      const;

 private:
  NativeMenuModel model_;
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
