#pragma once

#include "vulkan_platform_internal.hpp"

namespace cgpui {

enum class VulkanSwapchainOperation {
  acquire,
  present,
};

enum class VulkanSwapchainRecoveryTiming {
  none,
  before_frame,
  after_frame,
};

struct VulkanSwapchainRecoveryPlan {
  VulkanSwapchainRecoveryTiming recovery_timing =
      VulkanSwapchainRecoveryTiming::none;
  bool operation_succeeded = false;
  bool proceed_with_frame = false;
  bool retry_frame = false;
};

[[nodiscard]] VulkanSwapchainRecoveryPlan vulkan_plan_swapchain_recovery(
    VulkanSwapchainOperation operation,
    VkResult result);

} // namespace cgpui
