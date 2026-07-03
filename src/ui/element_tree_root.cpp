#include "cgpui/ui/element_tree.hpp"

#include <utility>

namespace cgpui {

ElementId ElementTree::set_root(std::unique_ptr<Element> element) {
  if (root_id_.value != 0) {
    remove_subtree(root_id_);
  }
  nodes_.clear();
  root_id_ = {};
  if (!element) {
    return {};
  }

  const ElementId id = allocate_id();
  element->assign_id(id);
  element->on_mount(ElementLifecycleContext{
      .element_id = id,
      .parent_element_id = std::nullopt,
  });
  nodes_.push_back(Node{
      .element = std::move(element),
      .id = id,
  });
  root_id_ = id;
  return id;
}

ElementId ElementTree::append_child(
    ElementId parent,
    std::unique_ptr<Element> element) {
  Node* parent_node = find_node(parent);
  if (parent_node == nullptr || !element) {
    return {};
  }

  const ElementId id = allocate_id();
  element->assign_id(id);
  element->on_mount(ElementLifecycleContext{
      .element_id = id,
      .parent_element_id = parent,
  });
  nodes_.push_back(Node{
      .element = std::move(element),
      .id = id,
      .parent = parent,
  });
  parent_node = find_node(parent);
  parent_node->children.push_back(id);
  return id;
}

ElementId ElementTree::reconcile_root(std::unique_ptr<Element> element) {
  if (!element) {
    return {};
  }
  if (root_id_.value == 0) {
    return set_root(std::move(element));
  }

  Node* root_node = find_node(root_id_);
  if (root_node == nullptr) {
    return set_root(std::move(element));
  }

  element->assign_id(root_id_);
  element->on_update(ElementLifecycleContext{
      .element_id = root_id_,
      .parent_element_id = std::nullopt,
  });
  root_node->element = std::move(element);
  return root_id_;
}

ElementId ElementTree::reconcile_child(
    ElementId parent,
    std::size_t index,
    std::unique_ptr<Element> element) {
  Node* parent_node = find_node(parent);
  if (parent_node == nullptr || !element) {
    return {};
  }

  if (index == parent_node->children.size()) {
    return append_child(parent, std::move(element));
  }
  if (index > parent_node->children.size()) {
    return {};
  }

  const ElementId child_id = parent_node->children[index];
  Node* child_node = find_node(child_id);
  if (child_node == nullptr) {
    return {};
  }

  element->assign_id(child_id);
  element->on_update(ElementLifecycleContext{
      .element_id = child_id,
      .parent_element_id = parent,
  });
  child_node->element = std::move(element);
  child_node->parent = parent;
  return child_id;
}

} // namespace cgpui
