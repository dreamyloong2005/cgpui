#include "vulkan_internal.hpp"

namespace cgpui {

VkResult VulkanRendererState::submit_frame(
    VkCommandBuffer command_buffer,
    VkSemaphore render_finished) {
  if (const VkResult result = vkResetFences(device_, 1, &in_flight_);
      result != VK_SUCCESS) {
    return result;
  }
  // Composition surfaces order the image with D3D11 by its keyed mutex.
  if (composition_ != nullptr) {
    return composition_->submit(graphics_queue_, command_buffer, in_flight_);
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
      .pSignalSemaphores = &render_finished,
  };
  return vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_);
}

} // namespace cgpui
