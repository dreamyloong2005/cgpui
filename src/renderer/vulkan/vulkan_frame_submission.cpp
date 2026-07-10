#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::submit_frame(VkCommandBuffer command_buffer) {
  if (auto result = require_vk_success(
          vkResetFences(device_, 1, &in_flight_), "vkResetFences failed");
      !result) {
    return result;
  }
  const VkPipelineStageFlags wait_stage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  const VkSubmitInfo submit_info{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &image_available_,
      .pWaitDstStageMask = &wait_stage,
      .commandBufferCount = 1,
      .pCommandBuffers = &command_buffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &render_finished_,
  };
  return require_vk_success(
      vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_),
      "vkQueueSubmit failed");
}

} // namespace cgpui
