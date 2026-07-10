#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::recover_swapchain_after_surface_status() {
  presentation_blocked_ = true;
  return resize(descriptor_.framebuffer_size, descriptor_.scale);
}

} // namespace cgpui
