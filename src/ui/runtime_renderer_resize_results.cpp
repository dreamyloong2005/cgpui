#include "ui_internal.hpp"

namespace cgpui {

Result<void> WindowRuntime::try_resize_surface(Size size, DpiScale scale) {
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
    return {};
  }

  return state_->windows.renderer_->resize(size, scale);
}

} // namespace cgpui
