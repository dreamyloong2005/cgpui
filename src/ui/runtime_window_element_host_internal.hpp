// Element tree of an additional window (a window runtime record). Records
// own and publish their tree the same way the root window uses
// owned_element_tree_: hit-test routing, cursors from the hit element,
// focus on press, layout, accessibility and window-control regions.
// Included inside WindowRuntime's private section.
struct WindowElementHost {
  std::unique_ptr<ElementTree> tree;
  bool view_rendered = false;
  std::unordered_map<std::uint64_t, CursorShape> cursors;
};
std::unordered_map<std::uint64_t, WindowElementHost> window_element_hosts_;
void set_element_tree_for_window(
    WindowRuntimeId runtime_id,
    std::unique_ptr<ElementTree> tree);
void set_element_cursor_for_window(
    WindowRuntimeId runtime_id,
    ElementId element_id,
    CursorShape cursor_shape);
[[nodiscard]] ElementTree* element_tree_for_record(
    const WindowRuntimeRecord& record) const;
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
void update_record_hover_cursor(
    WindowRuntimeRecord& record,
    const PlatformEvent& event,
    std::optional<ElementId> hit);
