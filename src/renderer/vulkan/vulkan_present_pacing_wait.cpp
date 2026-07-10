#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::wait_for_present_pacing() {
  if (!present_pacing_.wait_for_previous_frame) {
    return {};
  }
  return require_vk_success(
      vkWaitForFences(device_, 1, &in_flight_, VK_TRUE,
                      present_pacing_.fence_wait_timeout),
      "vkWaitForFences failed");
}

} // namespace cgpui
