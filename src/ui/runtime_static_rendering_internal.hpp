struct StaticRenderInstallResult {
  bool installed = false;
  ElementId root_id;
  std::size_t node_count = 0;
};
[[nodiscard]] StaticRenderInstallResult install_static_render_tree(
    View& view,
    ViewContext& render_context);
void set_static_element_tree(StaticElementTreeView tree);
void clear_static_element_tree();
[[nodiscard]] bool static_element_tree_installed() const;
[[nodiscard]] const StaticElementNode* static_element_node(
    ElementId element_id) const;
[[nodiscard]] bool element_enabled(ElementId element_id) const;
[[nodiscard]] bool element_focusable(ElementId element_id) const;
[[nodiscard]] std::optional<ElementId> element_parent(
    ElementId element_id) const;
[[nodiscard]] AccessibilityTreeSnapshot static_accessibility_snapshot(
    AccessibilitySnapshotOptions options) const;
[[nodiscard]] AccessibilityTreeSnapshot static_accessibility_snapshot_for(
    const StaticElementTreeView& tree,
    AccessibilitySnapshotOptions options) const;
