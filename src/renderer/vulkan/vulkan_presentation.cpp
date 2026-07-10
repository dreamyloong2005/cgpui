#include "vulkan_internal.hpp"

#include "vulkan_swapchain_recovery_policy_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::present_frame(
    Color color,
    std::span<const VulkanFrameDrawOrderEntry> draw_order,
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> text_selections,
    std::span<const TextCaretDraw> text_carets,
    std::span<const ImageDraw> image_draws,
    std::span<const ImageUploadBatch> image_uploads,
    std::span<const ImageAssetId> image_invalidations) {
  if (presentation_blocked_) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "Vulkan presentation requires swapchain recreation"));
  }

  last_command_batches_ = vulkan_build_renderer_command_batches(
      rects, rounded_rects, text_draws, text_selections, text_carets);
  if (auto result = wait_for_present_pacing(); !result) {
    return result;
  }
  if (auto result = prepare_solid_rect_frame(rects); !result) {
    return result;
  }
  if (auto result = prepare_rounded_rect_frame(rounded_rects); !result) {
    return result;
  }
  if (auto result = prepare_image_texture_frame(
          image_draws, image_uploads, image_invalidations);
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
      present_pacing_.image_acquire_timeout,
      image_available_,
      VK_NULL_HANDLE,
      &image_index);
  const VulkanSwapchainRecoveryPlan acquire_plan =
      vulkan_plan_swapchain_recovery(
          VulkanSwapchainOperation::acquire, acquire_result);
  if (acquire_plan.recovery_timing ==
      VulkanSwapchainRecoveryTiming::before_frame) {
    if (auto result = recover_swapchain_after_surface_status(); !result) {
      return result;
    }
    return std::unexpected(vulkan_error(
        ErrorCode::frame_acquisition_failed,
        "swapchain was recreated before frame acquisition; retry the frame"));
  }
  if (!acquire_plan.operation_succeeded) {
    return recover_after_failed_acquire(
        ErrorCode::frame_acquisition_failed,
        "vkAcquireNextImageKHR failed",
        false);
  }
  const bool recreate_after_frame =
      acquire_plan.recovery_timing ==
      VulkanSwapchainRecoveryTiming::after_frame;

  if (image_index >= command_buffers_.size() ||
      image_index >= command_reuse_states_.size()) {
    return recover_after_failed_record(
        "acquired swapchain image has no command recording state");
  }

  if (auto result = record_vulkan_frame_command_buffer(
          command_buffers_[image_index],
          render_pass_,
          framebuffers_[image_index],
          swapchain_extent_,
          rounded_rect_pipeline_resources_,
          solid_rect_buffers_,
          rounded_rect_buffers_,
          text_pipeline_resources_, text_vertex_buffer_,
          image_pipeline_resources_, image_vertex_buffer_, image_draws,
          color,
          draw_order,
          glyph_atlas_resources_,
          glyph_atlas_draw_quads_, glyph_atlas_draw_bindings_,
          glyph_atlas_uploads_,
          image_texture_resources_, image_texture_uploads_,
          command_reuse_states_[image_index]);
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
  commit_image_texture_frame();
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
  const VulkanSwapchainRecoveryPlan present_plan =
      vulkan_plan_swapchain_recovery(
          VulkanSwapchainOperation::present, present_result);
  if (!present_plan.operation_succeeded &&
      present_plan.recovery_timing == VulkanSwapchainRecoveryTiming::none) {
    return recover_after_failed_present("vkQueuePresentKHR failed");
  }
  if (recreate_after_frame ||
      present_plan.recovery_timing ==
          VulkanSwapchainRecoveryTiming::after_frame) {
    if (auto result = recover_swapchain_after_surface_status(); !result) {
      return result;
    }
  }
  if (present_plan.retry_frame) {
    return std::unexpected(vulkan_error(
        ErrorCode::frame_acquisition_failed,
        "swapchain was recreated after presentation; retry the frame"));
  }

  return {};
}

} // namespace cgpui
