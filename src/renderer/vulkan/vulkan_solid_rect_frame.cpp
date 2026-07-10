#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::prepare_solid_rect_frame(
    std::span<const SolidRect> rects) {
  return vulkan_upload_solid_rect_buffers(
      physical_device_, device_, rects, solid_rect_buffers_);
}

} // namespace cgpui
