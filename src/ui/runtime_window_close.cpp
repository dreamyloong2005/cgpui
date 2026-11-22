#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::close_window_record_after_event(
    WindowRuntimeRecord& record, const PlatformEvent& event) {
  const bool close_policy_pending = record.window != nullptr &&
      record.window->close_request_state().pending;
  record_lifecycle_event_for_record(record, event);
  // A lifecycle callback may already have closed this window.
  if (!record.active || record.window == nullptr) return;
  if (close_policy_pending) {
    if (record.window->close_request_state().pending) {
      (void)record.window->resolve_close_request(PlatformWindowCloseResolution::accept);
    }
    if (!record.window->close_request_state().accepted) return;
  }
  cleanup_closed_additional_window(record);
}

} // namespace cgpui
