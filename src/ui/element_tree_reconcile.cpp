#include "cgpui/ui/element_tree.hpp"

#include <utility>

namespace cgpui {

std::vector<ElementId> ElementTree::reconcile_children(
    ElementId parent,
    std::vector<AnyElement> elements) {
  Node* parent_node = find_node(parent);
  if (parent_node == nullptr) {
    return {};
  }

  const std::vector<ElementId> old_children = parent_node->children;
  std::vector<ElementId> new_children;
  std::vector<ElementId> used_children;
  new_children.reserve(elements.size());
  used_children.reserve(elements.size());

  for (std::size_t index = 0; index < elements.size(); ++index) {
    AnyElement& element = elements[index];
    if (!element) {
      continue;
    }

    ElementId child_id = element->key().has_value()
        ? find_keyed_child(old_children, *element->key(), used_children)
        : find_unkeyed_child_at_index(old_children, index, used_children);

    if (child_id.value == 0) {
      child_id = allocate_id();
      element->assign_id(child_id);
      element->on_mount(ElementLifecycleContext{
          .element_id = child_id,
          .parent_element_id = parent,
      });
      nodes_.push_back(Node{
          .element = std::move(element),
          .id = child_id,
          .parent = parent,
      });
    } else {
      Node* child_node = find_node(child_id);
      if (child_node == nullptr) {
        child_id = allocate_id();
        element->assign_id(child_id);
        element->on_mount(ElementLifecycleContext{
            .element_id = child_id,
            .parent_element_id = parent,
        });
        nodes_.push_back(Node{
            .element = std::move(element),
            .id = child_id,
            .parent = parent,
        });
      } else {
        element->assign_id(child_id);
        element->on_update(ElementLifecycleContext{
            .element_id = child_id,
            .parent_element_id = parent,
        });
        child_node->element = std::move(element);
        child_node->parent = parent;
      }
    }

    new_children.push_back(child_id);
    used_children.push_back(child_id);
  }

  parent_node = find_node(parent);
  if (parent_node == nullptr) {
    return {};
  }
  parent_node->children = new_children;

  for (ElementId old_child : old_children) {
    if (!contains_id(new_children, old_child)) {
      remove_subtree(old_child);
    }
  }

  return new_children;
}

ElementId ElementTree::find_keyed_child(
    const std::vector<ElementId>& old_children,
    const ElementKey& key,
    const std::vector<ElementId>& used_children) const {
  for (ElementId child_id : old_children) {
    if (contains_id(used_children, child_id)) {
      continue;
    }
    const Node* child_node = find_node(child_id);
    if (child_node == nullptr || !child_node->element->key().has_value()) {
      continue;
    }
    if (*child_node->element->key() == key) {
      return child_id;
    }
  }
  return {};
}

ElementId ElementTree::find_unkeyed_child_at_index(
    const std::vector<ElementId>& old_children,
    std::size_t index,
    const std::vector<ElementId>& used_children) const {
  if (index >= old_children.size()) {
    return {};
  }
  const ElementId child_id = old_children[index];
  if (contains_id(used_children, child_id)) {
    return {};
  }
  const Node* child_node = find_node(child_id);
  if (child_node == nullptr || child_node->element->key().has_value()) {
    return {};
  }
  return child_id;
}

} // namespace cgpui
