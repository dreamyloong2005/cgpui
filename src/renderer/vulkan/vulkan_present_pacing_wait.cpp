#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::wait_for_present_pacing() {
  if (!present_pacing_.wait_for_previous_frame) {
    return {};
  }
  const VkResult result = vkWaitForFences(device_, 1, &in_flight_, VK_TRUE,
                                        present_pacing_.fence_wait_timeout);
  return recover_after_device_loss(result, "vkWaitForFences failed");
}

} // namespace cgpui
