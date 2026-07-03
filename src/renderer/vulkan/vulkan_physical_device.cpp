#include "vulkan_internal.hpp"

namespace cgpui {

Result<VulkanQueueFamilySearch> VulkanRendererState::find_queue_families(
    VkPhysicalDevice device) const {
  std::uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queue_family_count, nullptr);
  if (queue_family_count == 0) {
    return VulkanQueueFamilySearch{};
  }

  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queue_family_count, queue_families.data());

  VulkanQueueFamilies selected;
  for (std::uint32_t index = 0; index < queue_family_count; ++index) {
    const VkQueueFamilyProperties& properties = queue_families[index];
    if (!selected.has_graphics && properties.queueCount > 0 &&
        (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      selected.graphics = index;
      selected.has_graphics = true;
    }

    VkBool32 present_supported = VK_FALSE;
    if (auto result = require_vk_success(
            vkGetPhysicalDeviceSurfaceSupportKHR(
                device, index, surface_, &present_supported),
            "vkGetPhysicalDeviceSurfaceSupportKHR failed");
        !result) {
      return std::unexpected(result.error());
    }

    if (!selected.has_present && properties.queueCount > 0 &&
        present_supported == VK_TRUE) {
      selected.present = index;
      selected.has_present = true;
    }

    if (selected.has_graphics && selected.has_present) {
      return VulkanQueueFamilySearch{
          .families = selected,
          .suitable = true,
      };
    }
  }

  return VulkanQueueFamilySearch{.families = selected, .suitable = false};
}

Result<bool> VulkanRendererState::has_device_extension(
    VkPhysicalDevice device,
    const char* extension_name) const {
  std::uint32_t extension_count = 0;
  if (auto result = require_vk_success(
          vkEnumerateDeviceExtensionProperties(
              device, nullptr, &extension_count, nullptr),
          "vkEnumerateDeviceExtensionProperties failed");
      !result) {
    return std::unexpected(result.error());
  }

  std::vector<VkExtensionProperties> extensions(extension_count);
  if (auto result = require_vk_success(
          vkEnumerateDeviceExtensionProperties(
              device, nullptr, &extension_count, extensions.data()),
          "vkEnumerateDeviceExtensionProperties failed");
      !result) {
    return std::unexpected(result.error());
  }

  for (const auto& extension : extensions) {
    if (std::strcmp(extension.extensionName, extension_name) == 0) {
      return true;
    }
  }

  return false;
}

Result<void> VulkanRendererState::select_physical_device() {
  std::uint32_t device_count = 0;
  if (auto result = require_vk_success(
          vkEnumeratePhysicalDevices(instance_, &device_count, nullptr),
          "vkEnumeratePhysicalDevices failed");
      !result) {
    return result;
  }

  if (device_count == 0) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "No Vulkan physical devices found"));
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  if (auto result = require_vk_success(
          vkEnumeratePhysicalDevices(instance_, &device_count, devices.data()),
          "vkEnumeratePhysicalDevices failed");
      !result) {
    return result;
  }

  for (VkPhysicalDevice device : devices) {
    auto queue_families = find_queue_families(device);
    if (!queue_families) {
      return std::unexpected(queue_families.error());
    }
    if (!queue_families->suitable) {
      continue;
    }

    auto supports_swapchain =
        has_device_extension(device, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (!supports_swapchain) {
      return std::unexpected(supports_swapchain.error());
    }
    if (!*supports_swapchain) {
      continue;
    }

    physical_device_ = device;
    graphics_queue_family_ = queue_families->families.graphics;
    present_queue_family_ = queue_families->families.present;
    return {};
  }

  return std::unexpected(vulkan_error(
      ErrorCode::renderer_initialization_failed,
      "No Vulkan physical device supports graphics, present, and swapchain"));
}

} // namespace cgpui
