#include "vulkan_present_pacing_internal.hpp"

namespace cgpui {

VulkanPresentPacingPlan vulkan_plan_present_pacing(
    std::span<const VkPresentModeKHR> present_modes,
    std::uint32_t min_image_count,
    std::uint32_t max_image_count) {
  const auto mailbox =
      std::ranges::find(present_modes, VK_PRESENT_MODE_MAILBOX_KHR);
  const VkPresentModeKHR present_mode = mailbox != present_modes.end()
      ? VK_PRESENT_MODE_MAILBOX_KHR
      : VK_PRESENT_MODE_FIFO_KHR;

  std::uint32_t image_count = min_image_count;
  if (image_count < std::numeric_limits<std::uint32_t>::max()) {
    ++image_count;
  }
  if (max_image_count != 0 && max_image_count >= min_image_count &&
      image_count > max_image_count) {
    image_count = max_image_count;
  }

  return VulkanPresentPacingPlan{
      .present_mode = present_mode,
      .swapchain_image_count = image_count,
      .max_frames_in_flight = 1,
      .wait_for_previous_frame = true,
      .fence_wait_timeout = std::numeric_limits<std::uint64_t>::max(),
      .image_acquire_timeout = std::numeric_limits<std::uint64_t>::max(),
  };
}

} // namespace cgpui
