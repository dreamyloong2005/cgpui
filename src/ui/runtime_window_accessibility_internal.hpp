void update_platform_accessibility_tree_for_record(
    WindowRuntimeRecord& record,
    const AccessibilityTreeSnapshot& snapshot);
void reset_accessibility_state_for_record(WindowRuntimeRecord& record);
[[nodiscard]] bool handle_accessibility_action_event(
    const PlatformEvent& event,
    ViewId target_view_id,
    WindowRuntimeRecord* record = nullptr);
