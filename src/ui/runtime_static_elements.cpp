#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::set_static_element_tree(StaticElementTreeView tree) {
  state_->elements.owned_element_tree_.reset();
  state_->elements.view_rendered_element_tree_ = false;
  state_->elements.element_root_ = nullptr;
  state_->elements.static_element_tree_ = tree;
  state_->elements.has_static_element_tree_ = !tree.empty() && tree.valid();
  apply_focused_text_ime_placement();
}

void WindowRuntime::clear_static_element_tree() {
  state_->elements.static_element_tree_ = {};
  state_->elements.has_static_element_tree_ = false;
}

bool WindowRuntime::static_element_tree_installed() const {
  return state_->elements.has_static_element_tree_ && !state_->elements.static_element_tree_.empty() &&
         state_->elements.static_element_tree_.valid();
}

const StaticElementTreeView* WindowRuntime::static_element_tree() const {
  return static_element_tree_installed() ? &state_->elements.static_element_tree_ : nullptr;
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
  if (state_->elements.owned_element_tree_ != nullptr) {
    return state_->elements.owned_element_tree_->parent(element_id);
  }
  return {};
}

} // namespace cgpui
