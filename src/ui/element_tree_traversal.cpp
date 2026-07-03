#include "cgpui/ui/element_tree.hpp"

namespace cgpui {

ElementId ElementTree::root_id() const {
  return root_id_;
}

std::optional<ElementId> ElementTree::parent(ElementId id) const {
  const Node* node = find_node(id);
  return node == nullptr ? std::optional<ElementId>{} : node->parent;
}

std::span<const ElementId> ElementTree::children(ElementId id) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return {};
  }
  return node->children;
}

LayoutOutput ElementTree::layout_root(LayoutInput input) const {
  const Element* root = get(root_id_);
  if (root == nullptr) {
    return LayoutOutput{
        .size = constrain_size({}, input.constraints),
    };
  }
  return root->layout(input);
}

ElementId ElementTree::hit_test_root(Point point) const {
  const Element* root = get(root_id_);
  return root == nullptr ? ElementId{} : root->hit_test(point);
}

std::vector<ElementId> ElementTree::preorder_ids() const {
  std::vector<ElementId> ids;
  append_preorder_ids(root_id_, ids);
  return ids;
}

std::vector<ElementId> ElementTree::enabled_preorder_ids() const {
  std::vector<ElementId> ids;
  append_enabled_preorder_ids(root_id_, ids);
  return ids;
}

void ElementTree::append_preorder_ids(
    ElementId id,
    std::vector<ElementId>& ids) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return;
  }

  ids.push_back(id);
  for (ElementId child_id : node->children) {
    append_preorder_ids(child_id, ids);
  }
}

void ElementTree::append_enabled_preorder_ids(
    ElementId id,
    std::vector<ElementId>& ids) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return;
  }

  if (node->element->enabled()) {
    ids.push_back(id);
  }
  for (ElementId child_id : node->children) {
    append_enabled_preorder_ids(child_id, ids);
  }
}

} // namespace cgpui
