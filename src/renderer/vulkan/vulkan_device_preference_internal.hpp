#pragma once

#include "vulkan_device_internal.hpp"

namespace cgpui {

// A physical device that passed the graphics, present, and swapchain checks,
// with the composite alpha modes its surface reports.
struct VulkanDeviceCandidate {
  VkPhysicalDevice device = VK_NULL_HANDLE;
  VulkanQueueFamilies families;
  VkCompositeAlphaFlagsKHR composite_alpha = 0;
};

// Composite alpha modes the surface supports on `device`; 0 when the query
// fails, which only removes the device's transparency preference.
[[nodiscard]] VkCompositeAlphaFlagsKHR vulkan_surface_composite_alpha(
    VkPhysicalDevice device,
    VkSurfaceKHR surface);

// True when `candidate` should replace `current`. Devices are otherwise kept
// in enumeration order; a transparent surface prefers premultiplied alpha.
[[nodiscard]] bool vulkan_prefer_device_candidate(
    const VulkanDeviceCandidate& candidate,
    const VulkanDeviceCandidate& current,
    bool transparent_surface);

} // namespace cgpui
