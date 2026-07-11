#include "ui_internal.hpp"

#include <expected>
#include <string_view>

namespace cgpui {
namespace {

template <typename ResultType>
std::string service_unsupported_message(
    std::string_view service,
    const ResultType& result) {
  if (!result.error_message.empty()) {
    return result.error_message;
  }
  std::string message(service);
  message += " unsupported by ";
  message += result.backend.empty() ? "unknown backend" : result.backend;
  return message;
}

template <typename ResultType>
Result<ResultType> unsupported_service_result(
    std::string_view service,
    const ResultType& result) {
  return std::unexpected(Error{
      .code = ErrorCode::unsupported_platform,
      .message = service_unsupported_message(service, result),
  });
}

} // namespace

Result<NativeMessageDialogResult>
WindowRuntime::try_show_native_message_dialog(
    NativeMessageDialogOptions options) {
  NativeMessageDialogResult result =
      application_.show_native_message_dialog(std::move(options));
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::message_dialog,
      .backend = result.backend,
      .operation = "try-show-native-message-dialog",
      .supported = result.supported,
      .succeeded = result.accepted,
      .value_count = result.accepted ? 1U : 0U,
  });
  if (!result.supported) {
    return unsupported_service_result("native message dialog", result);
  }
  native_message_dialog_result_ = std::move(result);
  return native_message_dialog_result_;
}

const NativeMessageDialogResult&
WindowRuntime::native_message_dialog_result() const {
  return native_message_dialog_result_;
}

Result<PlatformOpenUrlResult> WindowRuntime::try_open_url(std::string url) {
  PlatformOpenUrlResult result = application_.open_url(std::move(url));
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::open_url,
      .backend = result.backend,
      .operation = "try-open-url",
      .supported = result.supported,
      .succeeded = result.opened,
      .value_count = result.opened ? 1U : 0U,
  });
  if (!result.supported) {
    return unsupported_service_result("open URL", result);
  }
  open_url_result_ = std::move(result);
  return open_url_result_;
}

const PlatformOpenUrlResult& WindowRuntime::open_url_result() const {
  return open_url_result_;
}

Result<PlatformReopenResult> WindowRuntime::try_request_reopen() {
  PlatformReopenResult result = application_.request_reopen();
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::reopen,
      .backend = result.backend,
      .operation = "try-request-reopen",
      .supported = result.supported,
      .succeeded = result.requested,
      .value_count = result.requested ? 1U : 0U,
  });
  if (!result.supported) {
    return unsupported_service_result("reopen", result);
  }
  reopen_result_ = std::move(result);
  return reopen_result_;
}

const PlatformReopenResult& WindowRuntime::reopen_result() const {
  return reopen_result_;
}

} // namespace cgpui
