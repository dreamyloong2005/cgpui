#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::capture_pointer(PointerCaptureOwner owner) {
  if (!is_valid_pointer_capture_owner(owner)) {
    return;
  }
  const bool was_captured = pointer_capture_owner_.has_value();
  pointer_capture_owner_ = owner;
  sync_root_input_record();
  if (!was_captured && window_ != nullptr) {
    window_->set_pointer_capture(true);
  }
}

void WindowRuntime::release_pointer(PointerCaptureOwner owner) {
  if (pointer_capture_owner_ != owner) {
    return;
  }
  pointer_capture_owner_.reset();
  sync_root_input_record();
  if (window_ != nullptr) {
    window_->set_pointer_capture(false);
  }
}

void WindowRuntime::handle_pointer_capture_changed(
    const PointerCaptureChanged& changed) {
  if (changed.captured) {
    return;
  }
  pointer_capture_owner_.reset();
  input_.pointer_down_element_id.reset();
  input_.dragging_element_id.reset();
  input_.dragging = false;
  const std::optional<ElementId> previous = active_element_id_;
  active_element_id_.reset();
  sync_root_input_record();
  request_style_state_invalidation(previous, active_element_id_);
}

} // namespace cgpui
