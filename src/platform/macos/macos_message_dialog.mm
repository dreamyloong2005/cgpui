#include "macos_platform_services_internal.hpp"

namespace cgpui {
namespace {

NSString* message_string(std::string_view value) {
  return [[NSString alloc] initWithBytes:value.data()
                                  length:value.size()
                                encoding:NSUTF8StringEncoding];
}

}  // namespace

void macos_configure_message_dialog(
    NSAlert* alert,
    const NativeMessageDialogOptions& options) {
  [alert setMessageText:message_string(options.title) ?: @""];
  [alert setInformativeText:message_string(options.message) ?: @""];
  switch (options.kind) {
    case NativeMessageDialogKind::information:
    case NativeMessageDialogKind::question:
      [alert setAlertStyle:NSAlertStyleInformational]; break;
    case NativeMessageDialogKind::warning:
      [alert setAlertStyle:NSAlertStyleWarning]; break;
    case NativeMessageDialogKind::error:
      [alert setAlertStyle:NSAlertStyleCritical]; break;
  }
  switch (options.buttons) {
    case NativeMessageDialogButtons::ok: [alert addButtonWithTitle:@"OK"]; break;
    case NativeMessageDialogButtons::ok_cancel:
      [alert addButtonWithTitle:@"OK"];
      [alert addButtonWithTitle:@"Cancel"];
      break;
    case NativeMessageDialogButtons::yes_no:
      [alert addButtonWithTitle:@"Yes"];
      [alert addButtonWithTitle:@"No"];
      break;
  }
}

NativeMessageDialogResult macos_message_dialog_result(
    const NativeMessageDialogOptions& options,
    NSModalResponse response) {
  NativeMessageDialogResponse mapped = NativeMessageDialogResponse::none;
  if (response == NSAlertFirstButtonReturn) {
    mapped = options.buttons == NativeMessageDialogButtons::yes_no
        ? NativeMessageDialogResponse::yes : NativeMessageDialogResponse::ok;
  } else if (response == NSAlertSecondButtonReturn) {
    mapped = options.buttons == NativeMessageDialogButtons::yes_no
        ? NativeMessageDialogResponse::no : NativeMessageDialogResponse::cancel;
  }
  return NativeMessageDialogResult{
      .supported = true,
      .accepted = mapped == NativeMessageDialogResponse::ok ||
          mapped == NativeMessageDialogResponse::yes,
      .backend = "macos",
      .response = mapped};
}

NativeMessageDialogResult macos_show_native_message_dialog(
    const NativeMessageDialogOptions& options) {
  NSAlert* alert = [[NSAlert alloc] init];
  macos_configure_message_dialog(alert, options);
  return macos_message_dialog_result(options, [alert runModal]);
}

}  // namespace cgpui
