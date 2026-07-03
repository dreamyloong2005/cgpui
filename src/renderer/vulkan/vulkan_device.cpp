#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::create_device() {
  const float queue_priority = 1.0F;
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  queue_create_infos.reserve(
      graphics_queue_family_ == present_queue_family_ ? 1U : 2U);

  queue_create_infos.push_back(VkDeviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = graphics_queue_family_,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
  });

  if (graphics_queue_family_ != present_queue_family_) {
    queue_create_infos.push_back(VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = present_queue_family_,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    });
  }

  const std::array<const char*, 1> device_extensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };
  const VkDeviceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount =
          static_cast<std::uint32_t>(queue_create_infos.size()),
      .pQueueCreateInfos = queue_create_infos.data(),
      .enabledExtensionCount =
          static_cast<std::uint32_t>(device_extensions.size()),
      .ppEnabledExtensionNames = device_extensions.data(),
  };

  if (auto result = require_vk_success(
          vkCreateDevice(physical_device_, &create_info, nullptr, &device_),
          "vkCreateDevice failed");
      !result) {
    return result;
  }

  vkGetDeviceQueue(device_, graphics_queue_family_, 0, &graphics_queue_);
  vkGetDeviceQueue(device_, present_queue_family_, 0, &present_queue_);
  return {};
}

} // namespace cgpui
