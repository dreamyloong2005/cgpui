#include "ui_internal.hpp"

namespace cgpui {

Result<void> WindowRuntime::try_resize_surface(Size size, DpiScale scale) {
  framebuffer_size_ = size;
  scale_ = scale;
  viewport_size_ = to_logical_pixels(framebuffer_size_, scale_);
  if (renderer_ == nullptr) {
    return {};
  }

  return renderer_->resize(size, scale);
}

} // namespace cgpui
