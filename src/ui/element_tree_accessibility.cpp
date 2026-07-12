#include "cgpui/ui/element_tree.hpp"

namespace cgpui {

AccessibilityTreeSnapshot ElementTree::accessibility_snapshot(
    AccessibilitySnapshotOptions options) const {
  AccessibilityTreeSnapshot snapshot{
      .root_element_id = root_id_,
  };
  append_accessibility_nodes(root_id_, options, snapshot.nodes);
  return snapshot;
}

void ElementTree::append_accessibility_nodes(
    ElementId id,
    const AccessibilitySnapshotOptions& options,
    std::vector<AccessibilityNode>& nodes) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return;
  }

  const Element& element = *node->element;
  const FocusMetadata focus_metadata = element.focus_metadata();
  nodes.push_back(AccessibilityNode{
      .element_id = id,
      .parent_element_id = node->parent,
      .role = element.accessibility_role(),
      .name = element.accessibility_name(),
      .text = element.accessibility_text(),
      .value = element.accessibility_value(),
      .patterns = element.accessibility_patterns(),
      .enabled = element.enabled(),
      .focusable = element.focusable(),
      .focused = options.focused_element_id == id,
      .tab_index = focus_metadata.tab_index,
      .focus_ring = focus_metadata.focus_ring,
      .bounds = element.layout_bounds(),
      .children = node->children,
  });

  for (ElementId child_id : node->children) {
    append_accessibility_nodes(child_id, options, nodes);
  }
}

} // namespace cgpui
