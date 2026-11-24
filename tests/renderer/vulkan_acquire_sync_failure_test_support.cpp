#include "vulkan_acquire_sync_failure_test_support.hpp"

namespace cgpui_test {
namespace {
struct State {
  PFN_vkDeviceWaitIdle wait_idle = nullptr;
  PFN_vkCreateSemaphore create_semaphore = nullptr;
  PFN_vkDestroySemaphore destroy_semaphore = nullptr;
  PFN_vkDestroyFence destroy_fence = nullptr;
  PFN_vkWaitForFences wait_fences = nullptr;
  VulkanAcquireSyncObservation observation;
  bool active = false;
} state;
VKAPI_ATTR VkResult VKAPI_CALL wait_idle(VkDevice device) {
  const auto result = state.wait_idle(device);
  if (result == VK_SUCCESS) ++state.observation.idle_successes;
  return result;
}
VKAPI_ATTR VkResult VKAPI_CALL create_semaphore(VkDevice device, const VkSemaphoreCreateInfo* info,
    const VkAllocationCallbacks* allocator, VkSemaphore* semaphore) {
  auto& o = state.observation;
  ++o.semaphore_attempts;
  if (!o.injected) {
    o.injected = true;
    o.create_before_idle = o.idle_successes == 0;
    *semaphore = VK_NULL_HANDLE;
    return VK_ERROR_OUT_OF_HOST_MEMORY;
  }
  return state.create_semaphore(device, info, allocator, semaphore);
}
VKAPI_ATTR void VKAPI_CALL destroy_semaphore(VkDevice device, VkSemaphore semaphore,
    const VkAllocationCallbacks* allocator) {
  if (semaphore) {
    ++state.observation.semaphore_destroys;
    if (!state.observation.idle_successes) state.observation.teardown_before_idle = true;
  }
  state.destroy_semaphore(device, semaphore, allocator);
}
VKAPI_ATTR VkResult VKAPI_CALL wait_fences(VkDevice device, std::uint32_t count,
    const VkFence* fences, VkBool32 all, std::uint64_t timeout) {
  ++state.observation.fence_waits;
  bool invalid = count && !fences;
  if (fences) for (std::uint32_t i = 0; i < count; ++i) invalid |= fences[i] == VK_NULL_HANDLE;
  if (invalid) {
    ++state.observation.null_fence_waits;
    // Detect regressions without passing an invalid handle to the real driver.
    return VK_ERROR_INITIALIZATION_FAILED;
  }
  return state.wait_fences(device, count, fences, all, timeout);
}
VKAPI_ATTR void VKAPI_CALL destroy_fence(VkDevice device, VkFence fence,
    const VkAllocationCallbacks* allocator) {
  if (fence) {
    ++state.observation.fence_destroys;
    if (!state.observation.idle_successes) state.observation.teardown_before_idle = true;
  }
  state.destroy_fence(device, fence, allocator);
}
} // namespace
VulkanAcquireSyncFailureHook::VulkanAcquireSyncFailureHook() {
  if (state.active) return;
  state = {.wait_idle = cgpui::vkDeviceWaitIdle, .create_semaphore = cgpui::vkCreateSemaphore,
      .destroy_semaphore = cgpui::vkDestroySemaphore, .destroy_fence = cgpui::vkDestroyFence,
      .wait_fences = cgpui::vkWaitForFences,
      .active = true};
  if (!state.wait_idle || !state.create_semaphore || !state.destroy_semaphore ||
      !state.destroy_fence || !state.wait_fences) {
    state = {}; return;
  }
  cgpui::vkDeviceWaitIdle = wait_idle;
  cgpui::vkCreateSemaphore = create_semaphore;
  cgpui::vkDestroySemaphore = destroy_semaphore;
  cgpui::vkDestroyFence = destroy_fence;
  cgpui::vkWaitForFences = wait_fences;
  installed_ = true;
}
VulkanAcquireSyncFailureHook::~VulkanAcquireSyncFailureHook() {
  if (!installed_) return;
  cgpui::vkDeviceWaitIdle = state.wait_idle;
  cgpui::vkCreateSemaphore = state.create_semaphore;
  cgpui::vkDestroySemaphore = state.destroy_semaphore;
  cgpui::vkDestroyFence = state.destroy_fence;
  cgpui::vkWaitForFences = state.wait_fences;
  state.active = false;
}
bool VulkanAcquireSyncFailureHook::installed() const { return installed_; }
VulkanAcquireSyncObservation VulkanAcquireSyncFailureHook::observation() const { return state.observation; }
} // namespace cgpui_test
