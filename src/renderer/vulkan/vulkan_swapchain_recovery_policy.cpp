#include "vulkan_swapchain_recovery_policy_internal.hpp"

namespace cgpui {

VulkanSwapchainRecoveryPlan vulkan_plan_swapchain_recovery(
    VulkanSwapchainOperation operation,
    VkResult result) {
  if (result == VK_SUCCESS) {
    return VulkanSwapchainRecoveryPlan{
        .operation_succeeded = true,
        .proceed_with_frame = operation == VulkanSwapchainOperation::acquire,
    };
  }
  if (result == VK_SUBOPTIMAL_KHR) {
    return VulkanSwapchainRecoveryPlan{
        .recovery_timing = VulkanSwapchainRecoveryTiming::after_frame,
        .operation_succeeded = true,
        .proceed_with_frame = operation == VulkanSwapchainOperation::acquire,
    };
  }
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return VulkanSwapchainRecoveryPlan{
        .recovery_timing =
            operation == VulkanSwapchainOperation::acquire
                ? VulkanSwapchainRecoveryTiming::before_frame
                : VulkanSwapchainRecoveryTiming::after_frame,
        .retry_frame = true,
    };
  }
  return {};
}

} // namespace cgpui
