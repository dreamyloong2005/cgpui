#include "vulkan_state_internal.hpp"
#include "vulkan_swapchain_recovery_policy_internal.hpp"

namespace cgpui {

Result<bool> VulkanRendererState::complete_frame_acquisition(VkResult acquired) {
  if (acquired == VK_ERROR_DEVICE_LOST)
    return std::unexpected(recover_after_device_loss(acquired,
        "vkAcquireNextImageKHR lost device").error());
  const auto plan = vulkan_plan_swapchain_recovery(VulkanSwapchainOperation::acquire, acquired);
  if (plan.recovery_timing == VulkanSwapchainRecoveryTiming::before_frame) {
    if (auto result = recover_swapchain_after_surface_status(); !result)
      return std::unexpected(result.error());
    return std::unexpected(vulkan_error(ErrorCode::frame_retry_required,
        "swapchain was recreated before frame acquisition; retry the frame"));
  }
  if (!plan.operation_succeeded)
    return std::unexpected(recover_after_failed_acquire(ErrorCode::frame_acquisition_failed,
        "vkAcquireNextImageKHR failed", false).error());
  return plan.recovery_timing == VulkanSwapchainRecoveryTiming::after_frame;
}

} // namespace cgpui
