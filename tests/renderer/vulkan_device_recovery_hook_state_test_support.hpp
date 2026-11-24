#pragma once
#include "vulkan_device_recovery_hooks_test_support.hpp"
#include <array>

namespace cgpui_test {
struct VulkanRecoveryHookState {
  cgpui::VulkanWindowsFunctions real;
  VulkanRecoveryObservation observed;
  std::array<VkDevice, 8> devices{};
  std::array<VkPhysicalDevice, 128> enumerated{};
  std::uint32_t enumeration_count = 0;
  VulkanLossSite site = VulkanLossSite::none;
  VulkanRebuildFailure rebuild_failure = VulkanRebuildFailure::none;
  VkResult injected_result = VK_ERROR_DEVICE_LOST;
  unsigned fence_calls = 0;
  bool active = false, armed = false;
};
VulkanRecoveryHookState& recovery_hook_state();
void install_recovery_lifetime_hooks();
void install_recovery_operation_hooks();
// Returns false when this is not the selected site. A matching injection is
// consumed only after the real healthy device has been proven quiescent.
bool simulate_vulkan_loss(VulkanLossSite site, VkResult& result);
} // namespace cgpui_test
