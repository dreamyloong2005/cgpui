#include "vulkan_device_recovery_hook_state_test_support.hpp"
#include <algorithm>

namespace cgpui_test {
namespace {
VKAPI_ATTR VkResult VKAPI_CALL enumerate_devices(VkInstance instance, std::uint32_t* count, VkPhysicalDevice* devices) {
  auto& state = recovery_hook_state();
  const auto result = state.real.vkEnumeratePhysicalDevices(instance, count, devices);
  if (devices && result == VK_SUCCESS) {
    state.enumeration_count = (std::min)(*count, static_cast<std::uint32_t>(state.enumerated.size()));
    state.observed.reliable = state.observed.reliable && *count <= state.enumerated.size();
    for (std::uint32_t i = 0; i < state.enumeration_count; ++i) state.enumerated[i] = devices[i];
  }
  return result;
}
VKAPI_ATTR VkResult VKAPI_CALL create_device(VkPhysicalDevice physical, const VkDeviceCreateInfo* info,
    const VkAllocationCallbacks* allocator, VkDevice* device) {
  auto& state = recovery_hook_state();
  ++state.observed.device_attempts;
  if (state.observed.injected && state.rebuild_failure == VulkanRebuildFailure::device) {
    ++state.observed.rebuild_failures;
    *device = VK_NULL_HANDLE;
    return VK_ERROR_OUT_OF_DEVICE_MEMORY;
  }
  const auto result = state.real.vkCreateDevice(physical, info, allocator, device);
  if (result == VK_SUCCESS) {
    ++state.observed.devices_created;
    ++state.observed.live_devices;
    if (!state.observed.initial_physical) {
      state.observed.initial_physical = physical;
      for (std::uint32_t i = 0; i < state.enumeration_count; ++i)
        if (state.enumerated[i] == physical) state.observed.initial_enumeration_index = i;
      state.observed.reliable = state.observed.reliable && state.observed.initial_enumeration_index.has_value();
    }
    state.observed.latest_physical = physical;
    for (auto& value : state.devices) if (!value) { value = *device; return result; }
    state.observed.reliable = false;
  }
  return result;
}
VKAPI_ATTR void VKAPI_CALL destroy_device(VkDevice device, const VkAllocationCallbacks* allocator) {
  auto& state = recovery_hook_state();
  state.real.vkDestroyDevice(device, allocator);
  if (!device) return;
  ++state.observed.devices_destroyed;
  for (auto& value : state.devices) if (value == device) {
    value = VK_NULL_HANDLE; --state.observed.live_devices; return;
  }
  state.observed.reliable = false;
}
VKAPI_ATTR VkResult VKAPI_CALL create_instance(const VkInstanceCreateInfo* info,
    const VkAllocationCallbacks* allocator, VkInstance* instance) {
  auto& state = recovery_hook_state();
  const auto result = state.real.vkCreateInstance(info, allocator, instance);
  state.observed.instances_created += result == VK_SUCCESS;
  return result;
}
VKAPI_ATTR void VKAPI_CALL destroy_instance(VkInstance instance, const VkAllocationCallbacks* allocator) {
  auto& state = recovery_hook_state();
  state.real.vkDestroyInstance(instance, allocator);
  state.observed.instances_destroyed += instance != VK_NULL_HANDLE;
}
VKAPI_ATTR VkResult VKAPI_CALL create_surface(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* info,
    const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface) {
  auto& state = recovery_hook_state();
  const auto result = state.real.vkCreateWin32SurfaceKHR(instance, info, allocator, surface);
  state.observed.surfaces_created += result == VK_SUCCESS;
  return result;
}
VKAPI_ATTR void VKAPI_CALL destroy_surface(VkInstance instance, VkSurfaceKHR surface,
    const VkAllocationCallbacks* allocator) {
  auto& state = recovery_hook_state();
  state.real.vkDestroySurfaceKHR(instance, surface, allocator);
  state.observed.surfaces_destroyed += surface != VK_NULL_HANDLE;
}
VKAPI_ATTR VkResult VKAPI_CALL create_swapchain(VkDevice device, const VkSwapchainCreateInfoKHR* info,
    const VkAllocationCallbacks* allocator, VkSwapchainKHR* swapchain) {
  auto& state = recovery_hook_state();
  if (state.observed.injected && state.rebuild_failure == VulkanRebuildFailure::swapchain) {
    ++state.observed.rebuild_failures; *swapchain = VK_NULL_HANDLE;
    return VK_ERROR_OUT_OF_DEVICE_MEMORY;
  }
  return state.real.vkCreateSwapchainKHR(device, info, allocator, swapchain);
}
}
void install_recovery_lifetime_hooks() {
  cgpui::vkEnumeratePhysicalDevices = enumerate_devices;
  cgpui::vkCreateDevice = create_device; cgpui::vkDestroyDevice = destroy_device;
  cgpui::vkCreateInstance = create_instance; cgpui::vkDestroyInstance = destroy_instance;
  cgpui::vkCreateWin32SurfaceKHR = create_surface; cgpui::vkDestroySurfaceKHR = destroy_surface;
  cgpui::vkCreateSwapchainKHR = create_swapchain;
}
} // namespace cgpui_test
