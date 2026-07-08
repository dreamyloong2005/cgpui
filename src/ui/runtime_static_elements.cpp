#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::set_static_element_tree(StaticElementTreeView tree) {
  owned_element_tree_.reset();
  element_root_ = nullptr;
  static_element_tree_ = tree;
  has_static_element_tree_ = !tree.empty() && tree.valid();
  apply_focused_text_ime_placement();
}

void WindowRuntime::clear_static_element_tree() {
  static_element_tree_ = {};
  has_static_element_tree_ = false;
}

bool WindowRuntime::static_element_tree_installed() const {
  return has_static_element_tree_ && !static_element_tree_.empty() &&
         static_element_tree_.valid();
}

const StaticElementTreeView* WindowRuntime::static_element_tree() const {
  return static_element_tree_installed() ? &static_element_tree_ : nullptr;
}

const StaticElementNode* WindowRuntime::static_element_node(
    ElementId element_id) const {
  const StaticElementTreeView* tree = static_element_tree();
  return tree == nullptr ? nullptr : tree->get(element_id);
}

bool WindowRuntime::element_enabled(ElementId element_id) const {
  if (const StaticElementNode* node = static_element_node(element_id);
      node != nullptr) {
    return node->enabled;
  }
  const Element* element = routed_element(element_id);
  return element != nullptr && element->enabled();
}

bool WindowRuntime::element_focusable(ElementId element_id) const {
  if (const StaticElementNode* node = static_element_node(element_id);
      node != nullptr) {
    return node->enabled && node->focusable;
  }
  const Element* element = routed_element(element_id);
  return element != nullptr && element->enabled() && element->focusable();
}

std::optional<ElementId> WindowRuntime::element_parent(
    ElementId element_id) const {
  if (const StaticElementNode* node = static_element_node(element_id);
      node != nullptr && node->parent_id.value != 0) {
    return node->parent_id;
  }
  if (owned_element_tree_ != nullptr) {
    return owned_element_tree_->parent(element_id);
  }
  return {};
}

} // namespace cgpui
