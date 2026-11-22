#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::set_element_root(const Element* element) {
  state_->elements.owned_element_tree_.reset();
  state_->elements.view_rendered_element_tree_ = false;
  clear_static_element_tree();
  state_->elements.element_root_ = element;
  apply_focused_text_ime_placement();
}

void WindowRuntime::set_element_tree(std::unique_ptr<ElementTree> tree) {
  state_->elements.owned_element_tree_ = std::move(tree);
  state_->elements.view_rendered_element_tree_ = false;
  clear_static_element_tree();
  state_->elements.element_root_ = nullptr;
  apply_focused_text_ime_placement();
}

const ElementTree* WindowRuntime::element_tree() const {
  return state_->elements.owned_element_tree_.get();
}

const Element* WindowRuntime::element_root() const {
  if (state_->elements.owned_element_tree_ != nullptr) {
    return state_->elements.owned_element_tree_->get(state_->elements.owned_element_tree_->root_id());
  }
  return state_->elements.element_root_;
}

} // namespace cgpui
