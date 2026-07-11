#pragma once

#include "cgpui/platform/platform_message_dialog.hpp"

#include <windows.h>

#include <string>

namespace cgpui {

struct Win32MessageDialogPlan {
  UINT flags = MB_OK | MB_ICONINFORMATION;
  std::wstring title;
  std::wstring message;
};

[[nodiscard]] Win32MessageDialogPlan win32_message_dialog_plan(
    const NativeMessageDialogOptions& options);
[[nodiscard]] NativeMessageDialogResponse win32_message_dialog_response(
    int response);
[[nodiscard]] NativeMessageDialogResult show_win32_native_message_dialog(
    const NativeMessageDialogOptions& options);

} // namespace cgpui
