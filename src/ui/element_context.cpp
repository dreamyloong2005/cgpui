#include "cgpui/ui/element_context.hpp"

#include "cgpui/ui/runtime_context.hpp"

namespace cgpui {

ElementId ElementContextCapability::element_id() const {
  return element_id_;
}

void ElementContextCapability::request_keyboard_focus() const {
  context_->request_keyboard_focus(element_id_);
}

void ElementContextCapability::release_keyboard_focus() const {
  context_->release_keyboard_focus(element_id_);
}

void ElementContextCapability::focus() const {
  context_->focus(element_id_);
}

void ElementContextCapability::blur() const {
  context_->blur(element_id_);
}

FocusHandle ElementContextCapability::focus_handle() const {
  return context_->focus_handle(element_id_);
}

void ElementContextCapability::capture_pointer() const {
  context_->capture_pointer(element_id_);
}

void ElementContextCapability::release_pointer() const {
  context_->release_pointer(element_id_);
}

void ElementContextCapability::set_cursor(CursorShape cursor_shape) const {
  context_->set_element_cursor(element_id_, cursor_shape);
}

ElementContextCapability WindowRuntimeContext::element_context(
    ElementId element_id) const {
  return ElementContextCapability(*this, element_id);
}

} // namespace cgpui
