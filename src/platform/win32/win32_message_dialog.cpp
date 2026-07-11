#include "win32_message_dialog_internal.hpp"

#include "win32_internal.hpp"

namespace cgpui {

Win32MessageDialogPlan win32_message_dialog_plan(
    const NativeMessageDialogOptions& options) {
  UINT flags = MB_SETFOREGROUND;
  switch (options.kind) {
    case NativeMessageDialogKind::information: flags |= MB_ICONINFORMATION; break;
    case NativeMessageDialogKind::warning: flags |= MB_ICONWARNING; break;
    case NativeMessageDialogKind::error: flags |= MB_ICONERROR; break;
    case NativeMessageDialogKind::question: flags |= MB_ICONQUESTION; break;
  }
  switch (options.buttons) {
    case NativeMessageDialogButtons::ok: flags |= MB_OK; break;
    case NativeMessageDialogButtons::ok_cancel: flags |= MB_OKCANCEL; break;
    case NativeMessageDialogButtons::yes_no: flags |= MB_YESNO; break;
  }
  return {.flags = flags, .title = widen(options.title),
          .message = widen(options.message)};
}

NativeMessageDialogResponse win32_message_dialog_response(int response) {
  switch (response) {
    case IDOK: return NativeMessageDialogResponse::ok;
    case IDCANCEL: return NativeMessageDialogResponse::cancel;
    case IDYES: return NativeMessageDialogResponse::yes;
    case IDNO: return NativeMessageDialogResponse::no;
    default: return NativeMessageDialogResponse::none;
  }
}

NativeMessageDialogResult show_win32_native_message_dialog(
    const NativeMessageDialogOptions& options) {
  const Win32MessageDialogPlan plan = win32_message_dialog_plan(options);
  const int native_response = MessageBoxW(
      nullptr, plan.message.c_str(), plan.title.c_str(), plan.flags);
  const NativeMessageDialogResponse response =
      win32_message_dialog_response(native_response);
  return NativeMessageDialogResult{
      .supported = true,
      .accepted = response == NativeMessageDialogResponse::ok ||
          response == NativeMessageDialogResponse::yes,
      .backend = "win32",
      .response = response,
      .error_message = native_response == 0 ? "MessageBoxW failed" : "",
  };
}

} // namespace cgpui
