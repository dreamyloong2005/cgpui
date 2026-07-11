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
  std::wstring suggested_name;
  std::vector<Win32FileDialogFilterPlan> filters;
};

template <typename T>
class Win32FileDialogComPtr {
 public:
  ~Win32FileDialogComPtr() { if (value_ != nullptr) value_->Release(); }
  T** put() { return &value_; }
  T* get() const { return value_; }
  T* operator->() const { return value_; }

 private:
  T* value_ = nullptr;
};

[[nodiscard]] std::optional<Win32FileDialogPlan> win32_file_dialog_plan(
    const NativeFileDialogOptions& options);
[[nodiscard]] NativeFileDialogResult show_win32_native_file_dialog(
    const NativeFileDialogOptions& options);
[[nodiscard]] NativeFileDialogResult show_win32_native_save_file_dialog(
    const NativeFileDialogOptions& options,
    const Win32FileDialogPlan& plan);
[[nodiscard]] std::string win32_file_dialog_hresult_message(
    const char* operation,
    HRESULT result);
[[nodiscard]] bool win32_file_dialog_append_shell_item_path(
    IShellItem& item,
    NativeFileDialogResult& result);

} // namespace cgpui
