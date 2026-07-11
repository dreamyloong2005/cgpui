#include "win32_message_dialog_internal.hpp"

int main() {
  const auto question = cgpui::win32_message_dialog_plan({
      .kind = cgpui::NativeMessageDialogKind::question,
      .buttons = cgpui::NativeMessageDialogButtons::yes_no,
      .title = "Delete file?",
      .message = "This cannot be undone.",
  });
  if ((question.flags & MB_ICONQUESTION) == 0 ||
      (question.flags & MB_YESNO) == 0 ||
      question.title != L"Delete file?" ||
      question.message != L"This cannot be undone.") return 1;
  if (cgpui::win32_message_dialog_response(IDYES) !=
          cgpui::NativeMessageDialogResponse::yes ||
      cgpui::win32_message_dialog_response(IDNO) !=
          cgpui::NativeMessageDialogResponse::no ||
      cgpui::win32_message_dialog_response(IDCANCEL) !=
          cgpui::NativeMessageDialogResponse::cancel ||
      cgpui::win32_message_dialog_response(0) !=
          cgpui::NativeMessageDialogResponse::none) return 2;
  return 0;
}
