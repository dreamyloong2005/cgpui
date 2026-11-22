#include "ui_internal.hpp"

#include <utility>

namespace cgpui {

SystemAppearance WindowRuntime::system_appearance() const {
  return state_->core.application_.system_appearance();
}

PlatformDpiAwarenessState WindowRuntime::dpi_awareness() const {
  return state_->core.application_.dpi_awareness();
}

void WindowRuntime::set_system_appearance_callback(
    SystemAppearanceCallback callback) {
  if (state_->core.shutting_down_) return;
  state_->platform.system_appearance_callback_ = std::move(callback);
  if (state_->platform.system_appearance_bridge_installed_) return;
  state_->platform.system_appearance_bridge_installed_ = true;
  state_->core.application_.set_system_appearance_callback(
      [this](const SystemAppearanceChange& change) {
        handle_system_appearance_change(change);
      });
  // Records whether the backend can deliver changes at all; the reason for
  // an unsupported backend is in SystemAppearance::unsupported_reason.
  const SystemAppearance appearance = state_->core.application_.system_appearance();
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::appearance,
      .backend = appearance.backend,
      .operation = "system-appearance-monitor",
      .supported = appearance.supported,
      .succeeded = appearance.supported,
  });
}

void WindowRuntime::handle_system_appearance_change(
    const SystemAppearanceChange& change) {
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::appearance,
      .backend = change.appearance.backend,
      .operation = "system-appearance-changed",
      .supported = change.appearance.supported,
      .succeeded = true,
      .value_count = change.changed ? 1U : 0U,
  });
  if (change.changed) request_render();
  if (!state_->platform.system_appearance_callback_) return;
  const SystemAppearanceCallback callback = state_->platform.system_appearance_callback_;
  callback(change);
}

} // namespace cgpui
