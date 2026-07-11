#pragma once

#include <string>

namespace cgpui {

enum class NativeMessageDialogKind { information, warning, error, question };
enum class NativeMessageDialogButtons { ok, ok_cancel, yes_no };
enum class NativeMessageDialogResponse { none, ok, cancel, yes, no };

struct NativeMessageDialogOptions {
  NativeMessageDialogKind kind = NativeMessageDialogKind::information;
  NativeMessageDialogButtons buttons = NativeMessageDialogButtons::ok;
  std::string title;
  std::string message;
};

struct NativeMessageDialogResult {
  bool supported = false;
  bool accepted = false;
  std::string backend;
  NativeMessageDialogResponse response = NativeMessageDialogResponse::none;
  std::string error_message;
};

} // namespace cgpui
