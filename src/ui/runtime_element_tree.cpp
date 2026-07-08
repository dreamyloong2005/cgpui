#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::set_element_root(const Element* element) {
  owned_element_tree_.reset();
  clear_static_element_tree();
  element_root_ = element;
  apply_focused_text_ime_placement();
}

void WindowRuntime::set_element_tree(std::unique_ptr<ElementTree> tree) {
  owned_element_tree_ = std::move(tree);
  clear_static_element_tree();
  element_root_ = nullptr;
  apply_focused_text_ime_placement();
}

const ElementTree* WindowRuntime::element_tree() const {
  return owned_element_tree_.get();
}

const Element* WindowRuntime::element_root() const {
  if (owned_element_tree_ != nullptr) {
    return owned_element_tree_->get(owned_element_tree_->root_id());
  }
  return element_root_;
}

void WindowRuntime::capture_pointer(PointerCaptureOwner owner) {
  if (is_valid_pointer_capture_owner(owner)) {
    pointer_capture_owner_ = owner;
  }
}

void WindowRuntime::release_pointer(PointerCaptureOwner owner) {
  if (pointer_capture_owner_ == owner) {
    pointer_capture_owner_.reset();
  }
}

} // namespace cgpui
