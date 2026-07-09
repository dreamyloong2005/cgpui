#pragma once

#include "vulkan_platform_internal.hpp"

namespace cgpui {

struct VulkanQueueFamilies {
  std::uint32_t graphics = 0;
  std::uint32_t present = 0;
  bool has_graphics = false;
  bool has_present = false;
};

struct VulkanQueueFamilySearch {
  VulkanQueueFamilies families;
  bool suitable = false;
};

Result<std::uint32_t> vulkan_find_memory_type(
    VkPhysicalDevice physical_device,
    std::uint32_t supported_types,
    VkMemoryPropertyFlags required_flags);

} // namespace cgpui
