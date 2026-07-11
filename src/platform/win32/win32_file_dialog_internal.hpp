#pragma once

#include "cgpui/platform/platform_file_dialog.hpp"

#include <shobjidl.h>

#include <optional>
#include <string>
#include <vector>

namespace cgpui {

struct Win32FileDialogFilterPlan {
  std::wstring name;
  std::wstring pattern;
};

struct Win32FileDialogPlan {
  FILEOPENDIALOGOPTIONS options = FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST;
  std::wstring title;
  std::wstring default_directory;
  std::vector<Win32FileDialogFilterPlan> filters;
};

[[nodiscard]] std::optional<Win32FileDialogPlan> win32_file_dialog_plan(
    const NativeFileDialogOptions& options);
[[nodiscard]] NativeFileDialogResult show_win32_native_file_dialog(
    const NativeFileDialogOptions& options);

} // namespace cgpui
