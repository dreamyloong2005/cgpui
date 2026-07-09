#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::prepare_rounded_rect_frame(
    std::span<const RoundedRectDraw> rounded_rects) {
  return vulkan_upload_rounded_rect_buffers(
      physical_device_,
      device_,
      rounded_rects,
      rounded_rect_buffers_);
}

} // namespace cgpui
