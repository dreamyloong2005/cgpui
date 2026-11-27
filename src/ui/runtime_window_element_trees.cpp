#include "ui_internal.hpp"
#include "runtime_window_title_bar_internal.hpp"

#include <utility>

namespace cgpui {

void WindowRuntime::set_element_tree_for_window(
    WindowRuntimeId runtime_id,
    std::unique_ptr<ElementTree> tree) {
  if (runtime_id == state_->windows.root_window_runtime_id_) {
    set_element_tree(std::move(tree));
    return;
  }
  if (find_window_runtime_record(runtime_id) == nullptr) {
    return;
  }
  WindowElementHost& host = state_->elements.window_element_hosts_[runtime_id.value];
  host.tree = std::move(tree);
  host.static_tree = {};
  host.view_rendered = false;
}

void WindowRuntime::set_element_cursor_for_window(
    WindowRuntimeId runtime_id,
    ElementId element_id,
    CursorShape cursor_shape) {
  if (runtime_id == state_->windows.root_window_runtime_id_) {
    set_element_cursor(element_id, cursor_shape);
    return;
  }
  if (element_id.value == 0 || find_window_runtime_record(runtime_id) == nullptr) {
    return;
  }
  state_->elements.window_element_hosts_[runtime_id.value].cursors[element_id.value] =
      cursor_shape;
}

ElementTree* WindowRuntime::element_tree_for_record(
    const WindowRuntimeRecord& record) const {
  const auto host = state_->elements.window_element_hosts_.find(record.runtime_id.value);
  return host == state_->elements.window_element_hosts_.end() ? nullptr : host->second.tree.get();
}

AccessibilityTreeSnapshot WindowRuntime::accessibility_snapshot_for_window(
    WindowRuntimeId runtime_id) const {
  if (runtime_id == state_->windows.root_window_runtime_id_) {
    return accessibility_snapshot();
  }
  const WindowRuntimeRecord* record = find_window_runtime_record(runtime_id);
  if (record != nullptr) {
    if (const auto* tree = static_tree_for_record(*record)) {
      return static_accessibility_snapshot_for(*tree, AccessibilitySnapshotOptions{
          .focused_element_id = record->input.keyboard_focus_element_owner});
    }
  }
  const ElementTree* tree =
      record == nullptr ? nullptr : element_tree_for_record(*record);
  if (tree == nullptr) {
    return {};
  }
  return tree->accessibility_snapshot(AccessibilitySnapshotOptions{
      .focused_element_id = record->input.keyboard_focus_element_owner,
  });
}

// Same rule as the root window: an element returned from render replaces
// the tree; an empty render drops a tree that came from an earlier render
// but keeps one the view installed (WidgetTreeBuilder::install).
void WindowRuntime::install_rendered_element_for_record(
    WindowRuntimeRecord& record,
    AnyElement rendered,
    RenderTreeKind& tree_kind,
    std::optional<ElementId>& root_id) {
  WindowElementHost* host = nullptr;
  if (const auto found = state_->elements.window_element_hosts_.find(record.runtime_id.value);
      found != state_->elements.window_element_hosts_.end()) {
    host = &found->second;
    host->static_tree = {};
  }
  if (rendered != nullptr) {
    auto tree = std::make_unique<ElementTree>();
    (void)tree->set_root(std::move(rendered));
    set_element_tree_for_window(record.runtime_id, std::move(tree));
    state_->elements.window_element_hosts_[record.runtime_id.value].view_rendered = true;
  } else if (host != nullptr && host->view_rendered) {
    host->tree.reset();
    host->view_rendered = false;
  }
  if (const ElementTree* tree = element_tree_for_record(record); tree != nullptr) {
    tree_kind = RenderTreeKind::dynamic_element_tree;
    root_id = tree->root_id();
  }
}

// Lays out the record's element tree in its viewport and publishes it
// (static renders publish their own snapshot instead).
void WindowRuntime::publish_element_tree_for_record(WindowRuntimeRecord& record) {
  ElementTree* tree = element_tree_for_record(record);
  if (tree == nullptr) {
    return;
  }
  (void)tree->layout_root(LayoutInput{
      .constraints = {.max_size = record.viewport_size},
      .scale = record.scale,
  });
  update_platform_accessibility_tree_for_record(
      record, accessibility_snapshot_for_window(record.runtime_id));
}

} // namespace cgpui
