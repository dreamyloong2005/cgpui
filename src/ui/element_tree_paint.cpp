#include "cgpui/ui/element_tree.hpp"

#include <algorithm>

namespace cgpui {

void ElementTree::paint(PaintList& paint_list) const {
  paint_subtree(root_id_, paint_list);
}

void ElementTree::paint_subtree(ElementId id, PaintList& paint_list) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return;
  }

  node->element->paint(paint_list);
  std::vector<ElementId> ordered_children(
      node->children.begin(),
      node->children.end());
  std::stable_sort(
      ordered_children.begin(),
      ordered_children.end(),
      [this](ElementId lhs, ElementId rhs) {
        const Node* lhs_node = find_node(lhs);
        const Node* rhs_node = find_node(rhs);
        const int lhs_z = lhs_node == nullptr ? 0 : lhs_node->element->z_order();
        const int rhs_z = rhs_node == nullptr ? 0 : rhs_node->element->z_order();
        return lhs_z < rhs_z;
      });
  for (ElementId child_id : ordered_children) {
    paint_subtree(child_id, paint_list);
  }
}

} // namespace cgpui
