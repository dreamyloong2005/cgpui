#pragma once

#include "vulkan_platform_internal.hpp"

namespace cgpui {

struct VulkanPresentPacingPlan {
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  std::uint32_t swapchain_image_count = 0;
  std::uint32_t max_frames_in_flight = 1;
  bool wait_for_previous_frame = true;
  std::uint64_t fence_wait_timeout =
      std::numeric_limits<std::uint64_t>::max();
  std::uint64_t image_acquire_timeout =
      std::numeric_limits<std::uint64_t>::max();
};

[[nodiscard]] VulkanPresentPacingPlan vulkan_plan_present_pacing(
    std::span<const VkPresentModeKHR> present_modes,
    std::uint32_t min_image_count,
    std::uint32_t max_image_count);

} // namespace cgpui
