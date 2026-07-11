#include "ui_internal.hpp"

namespace cgpui {

Result<void> WindowRuntime::try_resize_surface(Size size, DpiScale scale) {
  framebuffer_size_ = size;
  scale_ = scale;
  viewport_size_ = to_logical_pixels(framebuffer_size_, scale_);
  if (WindowRuntimeRecord* root_record =
          find_window_runtime_record(root_window_runtime_id_);
      root_record != nullptr) {
    root_record->framebuffer_size = framebuffer_size_;
    root_record->viewport_size = viewport_size_;
    root_record->scale = scale_;
  }
  if (renderer_ == nullptr) {
    return {};
  }

  return renderer_->resize(size, scale);
}

} // namespace cgpui
