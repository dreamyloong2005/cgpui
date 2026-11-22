// Declaration fragment included inside WindowRuntime. No runtime state.
[[nodiscard]] bool handle_native_menu_command_event(const PlatformEvent& event, ViewId target_view_id);
void update_platform_accessibility_tree();
[[nodiscard]] PlatformAccessibilityTreeUpdate build_platform_accessibility_update()
    const;
