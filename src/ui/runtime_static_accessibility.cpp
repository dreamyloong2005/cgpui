#include "ui_internal.hpp"

namespace cgpui {
namespace {

AccessibilityRole accessibility_role_for(StaticElementKind kind) {
  switch (kind) {
    case StaticElementKind::text:
      return AccessibilityRole::text;
    case StaticElementKind::image:
      return AccessibilityRole::image;
    case StaticElementKind::container:
    case StaticElementKind::row:
    case StaticElementKind::column:
    case StaticElementKind::custom:
      return AccessibilityRole::generic;
  }
  return AccessibilityRole::generic;
}

} // namespace

AccessibilityTreeSnapshot WindowRuntime::static_accessibility_snapshot(
    AccessibilitySnapshotOptions options) const {
  AccessibilityTreeSnapshot snapshot;
  const StaticElementTreeView* tree = static_element_tree();
  if (tree == nullptr) {
    return snapshot;
  }

  snapshot.root_element_id = tree->root_id();
  snapshot.nodes.reserve(tree->size());
  tree->for_each_preorder([&](const StaticElementNode& node) {
    std::vector<ElementId> children;
    const std::span<const ElementId> child_ids = tree->children(node.id);
    children.reserve(child_ids.size());
    for (ElementId child_id : child_ids) {
      children.push_back(child_id);
    }

    snapshot.nodes.push_back(AccessibilityNode{
        .element_id = node.id,
        .parent_element_id =
            node.parent_id.value == 0
                ? std::optional<ElementId>{}
                : std::optional<ElementId>{node.parent_id},
        .role = accessibility_role_for(node.kind),
        .name = std::string{node.text},
        .text = std::string{node.text},
        .enabled = node.enabled,
        .focusable = node.focusable,
        .focused = options.focused_element_id == node.id,
        .bounds = node.bounds,
        .children = std::move(children),
    });
  });
  return snapshot;
}

} // namespace cgpui
