#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

Result<void> WindowRuntime::try_resize_surface(Size size, DpiScale scale) {
  const RuntimeWindowCallbackScope callback_scope(*this);
  state_->windows.framebuffer_size_ = size;
  state_->windows.scale_ = scale;
  state_->windows.viewport_size_ = to_logical_pixels(state_->windows.framebuffer_size_, state_->windows.scale_);
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(state_->windows.root_window_runtime_id_);
      root_record != nullptr) {
    root_record->framebuffer_size = state_->windows.framebuffer_size_;
    root_record->viewport_size = state_->windows.viewport_size_;
    root_record->scale = state_->windows.scale_;
  }
  if (state_->windows.renderer_ == nullptr) {
    state_->windows.surface_resizes_.clear(state_->windows.root_window_runtime_id_);
    return {};
  }

  return state_->windows.surface_resizes_.request(state_->windows.root_window_runtime_id_,
      *state_->windows.renderer_, size, scale);
}

} // namespace cgpui
