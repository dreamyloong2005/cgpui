#include "ui_internal.hpp"

#include <expected>

namespace cgpui {
namespace {

std::string native_menu_unsupported_message(
    const PlatformMenuInstallationResult& result) {
  if (result.backend.empty()) {
    return "native menu unsupported by unknown backend";
  }
  return "native menu unsupported by " + result.backend;
}

std::string file_dialog_unsupported_message(
    const NativeFileDialogResult& result) {
  if (!result.error_message.empty()) {
    return result.error_message;
  }
  if (result.backend.empty()) {
    return "native file dialog unsupported by unknown backend";
  }
  return "native file dialog unsupported by " + result.backend;
}

} // namespace

Result<NativeMenuInstallation> WindowRuntime::try_install_native_menu(
    NativeMenuModel menu) {
  NativeMenuModel stored_menu = std::move(menu);
  const PlatformMenuInstallationResult platform_result =
      application_.install_native_menu(stored_menu);
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::menu,
      .backend = platform_result.backend,
      .operation = "try-install-native-menu",
      .supported = platform_result.supported,
      .succeeded = platform_result.supported,
      .value_count = platform_result.item_count,
  });

  if (!platform_result.supported) {
    return std::unexpected(Error{
        .code = ErrorCode::unsupported_platform,
        .message = native_menu_unsupported_message(platform_result),
    });
  }

  native_menu_installation_ = NativeMenuInstallation{
      .model = std::move(stored_menu),
      .platform = platform_result,
  };
  return native_menu_installation_;
}

Result<NativeFileDialogResult> WindowRuntime::try_show_native_file_dialog(
    NativeFileDialogOptions options) {
  NativeFileDialogResult result =
      application_.show_native_file_dialog(std::move(options));
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::file_dialog,
      .backend = result.backend,
      .operation = "try-show-native-file-dialog",
      .supported = result.supported,
      .succeeded = result.supported,
      .value_count = result.paths.size(),
  });

  if (!result.supported) {
    return std::unexpected(Error{
        .code = ErrorCode::unsupported_platform,
        .message = file_dialog_unsupported_message(result),
    });
  }

  native_file_dialog_result_ = std::move(result);
  return native_file_dialog_result_;
}

} // namespace cgpui
