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
    std::span<const ImageAssetId> image_invalidations, bool capture_requested) {
  if (presentation_blocked_) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "Vulkan presentation requires swapchain recreation"));
  }

  VulkanFrameDiagnosticTimer diagnostic_timer;
  if (auto result = wait_for_present_pacing(); !result) {
    return result;
  }
  diagnostic_timer.finish_stage(RendererFrameTimingStage::pacing_wait);
  last_command_batches_ = vulkan_build_renderer_command_batches(
      rects, rounded_rects, text_draws, text_selections, text_carets);
  pending_frame_upload_bytes_ = renderer_upload_byte_counts({}, image_uploads);
  if (auto result = prepare_solid_rect_frame(rects); !result) {
    return result;
  }
  if (auto result = prepare_rounded_rect_frame(rounded_rects); !result) {
    return result;
  }
  if (auto result = prepare_image_texture_frame(
          image_draws, image_uploads, image_invalidations); !result) {
    return result;
  }
  if (auto result = prepare_glyph_atlas_frame(text_draws); !result) {
    return result;
  }
  diagnostic_timer.finish_stage(RendererFrameTimingStage::resource_preparation);
  std::uint32_t image_index = 0;
  const VkResult acquire_result = vkAcquireNextImageKHR(
      device_, swapchain_, present_pacing_.image_acquire_timeout,
      image_available_, VK_NULL_HANDLE, &image_index);
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
  diagnostic_timer.finish_stage(RendererFrameTimingStage::image_acquisition);
  const bool recreate_after_frame = acquire_plan.recovery_timing ==
                                    VulkanSwapchainRecoveryTiming::after_frame;

  if (image_index >= command_buffers_.size() ||
      image_index >= command_reuse_states_.size() ||
      image_index >= swapchain_images_.size()) {
    return recover_after_failed_record(
        "acquired swapchain image has no command recording state");
  }
  auto pixel_capture = prepare_frame_pixel_capture(
      capture_requested, swapchain_images_[image_index]);
  if (!pixel_capture) {
    return recover_after_failed_record(pixel_capture.error().message);
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
          *pixel_capture,
          command_reuse_states_[image_index]);
      !result) {
    return recover_after_failed_record(result.error().message);
  }
  diagnostic_timer.finish_stage(RendererFrameTimingStage::command_recording);
  VulkanFrameDiagnosticResources diagnostic_resources =
      vulkan_build_frame_diagnostic_resources(
          draw_order, solid_rect_buffers_.draws, rounded_rect_buffers_.draws,
          glyph_atlas_draw_bindings_, image_draws, last_command_batches_);

  VkCommandBuffer command_buffer = command_buffers_[image_index];
  if (auto result = submit_frame(command_buffer); !result) {
    return recover_after_failed_submit(result.error().message);
  }
  diagnostic_timer.finish_stage(RendererFrameTimingStage::queue_submission);
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
  const VkResult present_result = vkQueuePresentKHR(present_queue_, &present_info);
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
  if (auto result = complete_frame_pixel_capture(*pixel_capture); !result) {
    return result;
  }
  diagnostic_timer.finish_stage(RendererFrameTimingStage::presentation);
  last_frame_diagnostic_snapshot_ = vulkan_build_frame_diagnostic_snapshot(
      draw_order.size(),
      last_command_batches_.size(),
      std::move(diagnostic_resources),
      pending_frame_upload_bytes_,
      diagnostic_timer.timings());
  has_frame_diagnostic_snapshot_ = true;

  return {};
}

} // namespace cgpui
