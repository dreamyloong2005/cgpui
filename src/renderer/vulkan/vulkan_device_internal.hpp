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

} // namespace cgpui
