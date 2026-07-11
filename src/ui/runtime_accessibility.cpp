#include "ui_internal.hpp"

namespace cgpui {

AccessibilityTreeSnapshot WindowRuntime::accessibility_snapshot() const {
  if (static_element_tree_installed()) {
    return static_accessibility_snapshot(AccessibilitySnapshotOptions{
        .focused_element_id = keyboard_focus_element_owner_,
    });
  }
  if (owned_element_tree_ == nullptr) {
    return {};
  }
  return owned_element_tree_->accessibility_snapshot(
      AccessibilitySnapshotOptions{
          .focused_element_id = keyboard_focus_element_owner_,
      });
}

void WindowRuntime::update_platform_accessibility_tree() {
  if (window_ == nullptr ||
      (owned_element_tree_ == nullptr && !static_element_tree_installed())) {
    return;
  }
  WindowRuntimeRecord* record =
      find_window_runtime_record(root_window_runtime_id_);
  if (record != nullptr) {
    update_platform_accessibility_tree_for_record(
        *record, accessibility_snapshot());
  }
}

PlatformAccessibilityTreeUpdate WindowRuntime::build_platform_accessibility_update() const {
  PlatformAccessibilityTreeUpdate update =
      platform_accessibility_update_from(accessibility_snapshot());
  if (last_platform_accessibility_update_.has_value()) {
    append_platform_accessibility_live_updates(
        update,
        *last_platform_accessibility_update_);
  }
  return update;
}

} // namespace cgpui
