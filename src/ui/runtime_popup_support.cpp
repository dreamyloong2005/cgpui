#include "runtime_popup_support_internal.hpp"

#include "runtime_popups_internal.hpp"
#include "cgpui/platform/platform_application.hpp"
#include "cgpui/renderer/renderer_frame.hpp"
#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

bool runtime_popup_windows_supported(
    const PlatformApplication& application, const Renderer* renderer) {
  return renderer != nullptr && application.popup_windows_supported() &&
         renderer->popup_windows_supported();
}

bool runtime_accept_popup_renderer(
    WindowRuntimeRecord& record, const PlatformApplication& application,
    const Renderer* renderer) {
  if (record.descriptor.kind != WindowKind::popup ||
      runtime_popup_windows_supported(application, renderer)) {
    return true;
  }
  record.native_window_error = Error{
      .code = ErrorCode::unsupported_platform,
      .message = "popup windows require platform and renderer support"};
  return false;
}

bool RuntimePopups::supported(const WindowRuntime& runtime) {
  const WindowRuntimeRecord* root =
      runtime.window_runtime_record(runtime.root_window_runtime_id());
  return runtime_popup_windows_supported(
      runtime.application_, root == nullptr ? nullptr : root->renderer);
}

} // namespace cgpui
