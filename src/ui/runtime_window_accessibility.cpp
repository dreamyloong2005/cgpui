#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::update_platform_accessibility_tree_for_record(
    WindowRuntimeRecord& record,
    const AccessibilityTreeSnapshot& snapshot) {
  if (!record.active || record.window == nullptr || snapshot.nodes.empty()) {
    return;
  }
  PlatformAccessibilityTreeUpdate update =
      platform_accessibility_update_from(snapshot);
  if (record.last_accessibility_update.has_value()) {
    append_platform_accessibility_live_updates(
        update, *record.last_accessibility_update);
  }
  record_platform_diagnostic(PlatformDiagnosticEvent{
      .kind = PlatformDiagnosticKind::accessibility,
      .backend = "runtime",
      .operation = "update-tree",
      .supported = true,
      .succeeded = true,
      .value_count = update.node_count,
  });
  record.last_accessibility_update = update;
  if (record.runtime_id == root_window_runtime_id_) {
    last_platform_accessibility_update_ = update;
  }
  record.window->update_accessibility_tree(std::move(update));
}

void WindowRuntime::reset_accessibility_state_for_record(
    WindowRuntimeRecord& record) {
  record.last_accessibility_update.reset();
  if (record.runtime_id == root_window_runtime_id_) {
    last_platform_accessibility_update_.reset();
  }
}

} // namespace cgpui
