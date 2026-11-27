// Declaration fragment included inside WindowRuntime. No runtime state.
void set_element_tree_for_window(
    WindowRuntimeId runtime_id,
    std::unique_ptr<ElementTree> tree);
void set_element_cursor_for_window(
    WindowRuntimeId runtime_id,
    ElementId element_id,
    CursorShape cursor_shape);
[[nodiscard]] ElementTree* element_tree_for_record(
    const WindowRuntimeRecord& record) const;
[[nodiscard]] const StaticElementTreeView* static_tree_for_record(
    const WindowRuntimeRecord& record) const;
void install_static_tree_for_record(
    WindowRuntimeRecord& record, StaticElementTreeView tree);
[[nodiscard]] EventResult dispatch_record_event_through_static_elements(
    WindowRuntimeRecord& record, View& view, const PlatformEvent& event);
[[nodiscard]] AccessibilityTreeSnapshot accessibility_snapshot_for_window(
    WindowRuntimeId runtime_id) const;
void install_rendered_element_for_record(
    WindowRuntimeRecord& record,
    AnyElement rendered,
    RenderTreeKind& tree_kind,
    std::optional<ElementId>& root_id);
void publish_element_tree_for_record(WindowRuntimeRecord& record);
[[nodiscard]] EventResult dispatch_record_event_through_elements(
    WindowRuntimeRecord& record,
    View& view,
    const PlatformEvent& event);
[[nodiscard]] std::optional<ElementId> update_record_hover_cursor(
    WindowRuntimeRecord& record,
    const PlatformEvent& event);
