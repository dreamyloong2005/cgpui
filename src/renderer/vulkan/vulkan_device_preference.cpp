#include "vulkan_device_preference_internal.hpp"

namespace cgpui {
namespace {

bool supports_premultiplied(const VulkanDeviceCandidate& candidate) {
  return (candidate.composite_alpha &
          VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) != 0;
}

} // namespace

VkCompositeAlphaFlagsKHR vulkan_surface_composite_alpha(
    VkPhysicalDevice device,
    VkSurfaceKHR surface) {
  VkSurfaceCapabilitiesKHR capabilities{};
  if (surface == VK_NULL_HANDLE ||
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          device, surface, &capabilities) != VK_SUCCESS) {
    return 0;
  }
  return capabilities.supportedCompositeAlpha;
}

bool vulkan_prefer_device_candidate(
    const VulkanDeviceCandidate& candidate,
    const VulkanDeviceCandidate& current,
    bool transparent_surface) {
  return transparent_surface && supports_premultiplied(candidate) &&
         !supports_premultiplied(current);
}

} // namespace cgpui
