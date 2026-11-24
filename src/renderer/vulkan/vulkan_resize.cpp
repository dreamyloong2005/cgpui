#include "vulkan_internal.hpp"

namespace cgpui {

DpiScale VulkanRendererState::scale() const { return descriptor_.scale; }

Result<void> VulkanRendererState::resize(
    Size framebuffer_size,
    DpiScale scale) {
  if (framebuffer_size.width <= 0.0F || framebuffer_size.height <= 0.0F) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a non-empty framebuffer",
    });
  }
  if (auto result = require_live_device(); !result) return result;

  const RenderSurfaceDescriptor previous_descriptor = descriptor_;
  descriptor_.framebuffer_size = framebuffer_size;
  descriptor_.scale = scale;
  const VkResult idle = vkDeviceWaitIdle(device_);
  if (idle != VK_SUCCESS) {
    const auto generation = device_generation_;
    // Resize has no acquired frame to retry. Rebuild against the requested
    // descriptor, and report success only after that complete device is live.
    auto recovered = recover_after_device_loss(idle, "vkDeviceWaitIdle before resize failed");
    if (idle == VK_ERROR_DEVICE_LOST && device_generation_ != generation &&
        !recovered && recovered.error().code == ErrorCode::frame_retry_required) return {};
    descriptor_ = previous_descriptor;
    return recovered;
  }
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

  presentation_blocked_ = true;
  destroy_swapchain();
  install_swapchain(std::move(*resources));
  const auto synchronized = image_available_ == VK_NULL_HANDLE || in_flight_ == VK_NULL_HANDLE
      ? create_sync_objects() : create_render_finished_semaphores();
  if (!synchronized) {
    presentation_blocked_ = true;
    return synchronized;
  }
  presentation_blocked_ = false;
  return {};
}

} // namespace cgpui
