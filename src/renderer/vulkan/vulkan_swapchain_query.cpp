#include "vulkan_internal.hpp"

namespace cgpui {

Result<VulkanSwapchainSurfaceDetails> query_vulkan_swapchain_surface(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface) {
  VulkanSwapchainSurfaceDetails details;
  if (auto result = require_vk_success(
          vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
              physical_device,
              surface,
              &details.capabilities),
          "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
      !result) {
    return std::unexpected(result.error());
  }

  if ((details.capabilities.supportedUsageFlags &
       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "Vulkan surface does not support color-attachment swapchain images"));
  }

  std::uint32_t format_count = 0;
  if (auto result = require_vk_success(
          vkGetPhysicalDeviceSurfaceFormatsKHR(
              physical_device,
              surface,
              &format_count,
              nullptr),
          "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
      !result) {
    return std::unexpected(result.error());
  }
  if (format_count == 0) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "No Vulkan surface formats available"));
  }

  details.formats.resize(format_count);
  if (auto result = require_vk_success(
          vkGetPhysicalDeviceSurfaceFormatsKHR(
              physical_device,
              surface,
              &format_count,
              details.formats.data()),
          "vkGetPhysicalDeviceSurfaceFormatsKHR failed");
      !result) {
    return std::unexpected(result.error());
  }

  std::uint32_t present_mode_count = 0;
  if (auto result = require_vk_success(
          vkGetPhysicalDeviceSurfacePresentModesKHR(
              physical_device,
              surface,
              &present_mode_count,
              nullptr),
          "vkGetPhysicalDeviceSurfacePresentModesKHR failed");
      !result) {
    return std::unexpected(result.error());
  }
  if (present_mode_count == 0) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "No Vulkan surface present modes available"));
  }

  details.present_modes.resize(present_mode_count);
  if (auto result = require_vk_success(
          vkGetPhysicalDeviceSurfacePresentModesKHR(
              physical_device,
              surface,
              &present_mode_count,
              details.present_modes.data()),
          "vkGetPhysicalDeviceSurfacePresentModesKHR failed");
      !result) {
    return std::unexpected(result.error());
  }

  return details;
}

} // namespace cgpui
