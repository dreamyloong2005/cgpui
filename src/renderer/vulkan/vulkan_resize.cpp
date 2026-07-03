#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::resize(
    Size framebuffer_size,
    DpiScale scale) {
  if (framebuffer_size.width <= 0.0F || framebuffer_size.height <= 0.0F) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a non-empty framebuffer",
    });
  }

  const RenderSurfaceDescriptor previous_descriptor = descriptor_;
  descriptor_.framebuffer_size = framebuffer_size;
  descriptor_.scale = scale;
  vkDeviceWaitIdle(device_);
  bool retired_old_swapchain = false;
  auto resources =
      create_swapchain_resources(swapchain_, &retired_old_swapchain);
  if (!resources) {
    descriptor_ = previous_descriptor;
    if (retired_old_swapchain) {
      destroy_swapchain();
      presentation_blocked_ = true;
    }
    return std::unexpected(resources.error());
  }

  destroy_swapchain();
  install_swapchain(std::move(*resources));
  presentation_blocked_ = false;
  return {};
}

} // namespace cgpui
