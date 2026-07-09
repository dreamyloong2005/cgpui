#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::present_frame(
    Color color,
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> text_selections,
    std::span<const TextCaretDraw> text_carets) {
  if (presentation_blocked_) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "Vulkan presentation requires swapchain recreation"));
  }

  last_command_batches_ = vulkan_build_renderer_command_batches(
      rects,
      rounded_rects,
      text_draws,
      text_selections,
      text_carets);
  if (auto result = require_vk_success(
          vkWaitForFences(device_, 1, &in_flight_, VK_TRUE, UINT64_MAX),
          "vkWaitForFences failed");
      !result) {
    return result;
  }
  if (auto result = prepare_glyph_atlas_frame(text_draws); !result) {
    return result;
  }

  std::uint32_t image_index = 0;
  const VkResult acquire_result = vkAcquireNextImageKHR(
      device_,
      swapchain_,
      UINT64_MAX,
      image_available_,
      VK_NULL_HANDLE,
      &image_index);
  if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
    return recover_after_failed_acquire(
        ErrorCode::renderer_initialization_failed,
        "vkAcquireNextImageKHR reported an out-of-date swapchain",
        true);
  }
  if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
    return recover_after_failed_acquire(
        ErrorCode::frame_acquisition_failed,
        "vkAcquireNextImageKHR failed",
        false);
  }
  const bool acquired_suboptimal = acquire_result == VK_SUBOPTIMAL_KHR;

  if (auto result = record_vulkan_frame_command_buffer(
          command_buffers_[image_index],
          render_pass_,
          framebuffers_[image_index],
          swapchain_extent_,
          color,
          rects,
          glyph_atlas_resources_,
          glyph_atlas_uploads_);
      !result) {
    return recover_after_failed_record(result.error().message);
  }

  if (auto result = require_vk_success(
          vkResetFences(device_, 1, &in_flight_), "vkResetFences failed");
      !result) {
    return recover_after_failed_record(result.error().message);
  }

  VkCommandBuffer command_buffer = command_buffers_[image_index];

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
  if (auto result = require_vk_success(
          vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_),
          "vkQueueSubmit failed");
      !result) {
    return recover_after_failed_submit(result.error().message);
  }
  commit_glyph_atlas_frame();

  const VkPresentInfoKHR present_info{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &render_finished_,
      .swapchainCount = 1,
      .pSwapchains = &swapchain_,
      .pImageIndices = &image_index,
  };
  const VkResult present_result =
      vkQueuePresentKHR(present_queue_, &present_info);
  if (present_result == VK_ERROR_OUT_OF_DATE_KHR ||
      present_result == VK_SUBOPTIMAL_KHR) {
    presentation_blocked_ = true;
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "vkQueuePresentKHR reported an out-of-date swapchain"));
  }
  if (auto result =
          require_vk_success(present_result, "vkQueuePresentKHR failed");
      !result) {
    return recover_after_failed_present(result.error().message);
  }
  if (acquired_suboptimal) {
    presentation_blocked_ = true;
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "vkAcquireNextImageKHR reported a suboptimal swapchain"));
  }

  return {};
}

} // namespace cgpui
