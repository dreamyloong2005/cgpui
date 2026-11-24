#include "vulkan_device_recovery_hook_state_test_support.hpp"

namespace cgpui_test {
namespace {
VKAPI_ATTR VkResult VKAPI_CALL acquire(VkDevice device, VkSwapchainKHR chain, std::uint64_t timeout,
    VkSemaphore semaphore, VkFence fence, std::uint32_t* index) {
  VkResult result;
  if (simulate_vulkan_loss(VulkanLossSite::acquire, result)) return result;
  return recovery_hook_state().real.vkAcquireNextImageKHR(device, chain, timeout, semaphore, fence, index);
}
VKAPI_ATTR VkResult VKAPI_CALL submit(VkQueue queue, std::uint32_t count,
    const VkSubmitInfo* submits, VkFence fence) {
  VkResult result;
  if (simulate_vulkan_loss(VulkanLossSite::submit, result)) return result;
  return recovery_hook_state().real.vkQueueSubmit(queue, count, submits, fence);
}
VKAPI_ATTR VkResult VKAPI_CALL present(VkQueue queue, const VkPresentInfoKHR* info) {
  VkResult result;
  if (simulate_vulkan_loss(VulkanLossSite::present, result)) return result;
  return recovery_hook_state().real.vkQueuePresentKHR(queue, info);
}
VKAPI_ATTR VkResult VKAPI_CALL wait_fences(VkDevice device, std::uint32_t count,
    const VkFence* fences, VkBool32 all, std::uint64_t timeout) {
  auto& state = recovery_hook_state();
  if (state.armed) {
    ++state.fence_calls;
    VkResult result;
    const auto site = state.fence_calls == 1 ? VulkanLossSite::pacing_wait : VulkanLossSite::capture_wait;
    if (simulate_vulkan_loss(site, result)) return result;
  }
  return state.real.vkWaitForFences(device, count, fences, all, timeout);
}
VKAPI_ATTR VkResult VKAPI_CALL wait_idle(VkDevice device) {
  VkResult result;
  if (simulate_vulkan_loss(VulkanLossSite::resize_wait, result)) return result;
  return recovery_hook_state().real.vkDeviceWaitIdle(device);
}
}
void install_recovery_operation_hooks() {
  cgpui::vkAcquireNextImageKHR = acquire; cgpui::vkQueueSubmit = submit;
  cgpui::vkQueuePresentKHR = present; cgpui::vkWaitForFences = wait_fences;
  cgpui::vkDeviceWaitIdle = wait_idle;
}
} // namespace cgpui_test
