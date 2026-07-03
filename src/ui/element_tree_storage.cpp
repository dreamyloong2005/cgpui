#include "cgpui/ui/element_tree.hpp"

#include <algorithm>

namespace cgpui {

Element* ElementTree::get(ElementId id) {
  Node* node = find_node(id);
  return node == nullptr ? nullptr : node->element.get();
}

const Element* ElementTree::get(ElementId id) const {
  const Node* node = find_node(id);
  return node == nullptr ? nullptr : node->element.get();
}

ElementId ElementTree::allocate_id() {
  const ElementId id{next_id_};
  next_id_ += 1;
  return id;
}

bool ElementTree::contains_id(
    const std::vector<ElementId>& ids,
    ElementId id) {
  return std::find(ids.begin(), ids.end(), id) != ids.end();
}

ElementTree::Node* ElementTree::find_node(ElementId id) {
  for (Node& node : nodes_) {
    if (node.id == id) {
      return &node;
    }
  }
  return nullptr;
}

const ElementTree::Node* ElementTree::find_node(ElementId id) const {
  for (const Node& node : nodes_) {
    if (node.id == id) {
      return &node;
    }
  }
  return nullptr;
}

void ElementTree::remove_subtree(ElementId id) {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return;
  }

  const std::vector<ElementId> children = node->children;
  for (ElementId child_id : children) {
    remove_subtree(child_id);
  }
  node = find_node(id);
  if (node != nullptr) {
    node->element->on_unmount(ElementLifecycleContext{
        .element_id = id,
        .parent_element_id = node->parent,
    });
  }
  nodes_.erase(
      std::remove_if(
          nodes_.begin(),
          nodes_.end(),
          [id](const Node& candidate) {
            return candidate.id == id;
          }),
      nodes_.end());
}

} // namespace cgpui
