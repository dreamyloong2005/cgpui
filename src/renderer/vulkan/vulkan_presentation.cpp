#include "vulkan_internal.hpp"

#include "vulkan_swapchain_recovery_policy_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::present_frame(
    Color color,
    std::span<const VulkanFrameDrawOrderEntry> draw_order,
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const BoxShadowDraw> box_shadows,
    std::span<const BackdropBlurDraw> backdrops,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> text_selections,
    std::span<const TextCaretDraw> text_carets,
    std::span<const ImageDraw> image_draws,
    std::span<ImageUploadBatch> image_uploads,
    std::span<const ImageAssetId> image_invalidations, bool capture_requested) {
  if (auto result = require_live_device(); !result) return result;
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
  last_command_batches_ = build_renderer_command_batches(
      rects, rounded_rects, text_draws, text_selections, text_carets);
  pending_frame_upload_bytes_ = renderer_upload_byte_counts({}, image_uploads);
  if (auto result = prepare_solid_rect_frame(rects); !result) {
    return result;
  }
  if (auto result = prepare_rounded_rect_frame(rounded_rects); !result) {
    return result;
  }
  if (auto result = prepare_effect_frame(box_shadows, backdrops); !result) {
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
  // A composition surface has one image, ordered by its keyed mutex.
  const VkResult acquire_result = composition_ != nullptr
      ? VK_SUCCESS
      : vkAcquireNextImageKHR(device_, swapchain_, present_pacing_.image_acquire_timeout,
                              image_available_, VK_NULL_HANDLE, &image_index);
  const auto acquired = complete_frame_acquisition(acquire_result);
  if (!acquired) return std::unexpected(acquired.error());
  diagnostic_timer.finish_stage(RendererFrameTimingStage::image_acquisition);
  const bool recreate_after_frame = *acquired;

  if (image_index >= command_buffers_.size() ||
      image_index >= command_reuse_states_.size() ||
      image_index >= swapchain_images_.size() ||
      image_index >= render_finished_.size()) {
    return recover_after_failed_record(
        "acquired swapchain image has no command recording state");
  }
  auto pixel_capture = prepare_frame_pixel_capture(
      capture_requested, swapchain_images_[image_index]);
  if (!pixel_capture) {
    return recover_after_failed_record(pixel_capture.error().message);
  }

  auto recording = record_vulkan_frame_command_buffer(
      {.target = {.command_buffer = command_buffers_[image_index],
                  .render_pass = render_pass_, .framebuffer = framebuffers_[image_index],
                  .extent = swapchain_extent_, .clear_color = color},
       .geometry = {.pipeline = rounded_rect_pipeline_resources_,
                    .solid_rect_buffers = solid_rect_buffers_,
                    .rounded_rect_buffers = rounded_rect_buffers_},
       .text = {.pipeline = text_pipeline_resources_, .vertex_buffer = text_vertex_buffer_,
                .glyph_atlas = glyph_atlas_resources_,
                .glyph_atlas_draw_quads = glyph_atlas_draw_quads_,
                .glyph_atlas_draw_bindings = glyph_atlas_draw_bindings_,
                .uploads = glyph_atlas_uploads_},
       .images = {.pipeline = image_pipeline_resources_, .vertex_buffer = image_vertex_buffer_,
                  .draws = image_draws, .textures = image_texture_resources_,
                  .uploads = image_texture_uploads_},
       .draw_order = draw_order, .pixel_capture = *pixel_capture,
       .effects = frame_effects()},
      command_reuse_states_[image_index]);
  if (!recording) {
    return recover_after_failed_record(recording.error().message);
  }
  diagnostic_timer.finish_stage(RendererFrameTimingStage::command_recording);
  VulkanFrameDiagnosticResources diagnostic_resources =
      vulkan_build_frame_diagnostic_resources(
          draw_order, solid_rect_buffers_.draws, rounded_rect_buffers_.draws,
          glyph_atlas_draw_bindings_, recording->submitted_image_indices, last_command_batches_,
          box_shadow_buffers_.draws, backdrop_blur_plan_.passes);

  VkCommandBuffer command_buffer = command_buffers_[image_index];
  VkSemaphore render_finished = render_finished_[image_index];
  if (const VkResult result = submit_frame(command_buffer, render_finished);
      result != VK_SUCCESS) {
    if (result == VK_ERROR_DEVICE_LOST)
      return recover_after_device_loss(result, "Vulkan frame submission lost device");
    return recover_after_failed_submit("Vulkan frame submission failed");
  }
  diagnostic_timer.finish_stage(RendererFrameTimingStage::queue_submission);
  commit_image_texture_frame();
  commit_glyph_atlas_frame();

  VkResult present_result = VK_SUCCESS;
  if (composition_ != nullptr) {
    if (auto result = composition_->present(); !result) {
      return recover_after_failed_present(result.error().message);
    }
  } else {
    const VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_finished,
        .swapchainCount = 1,
        .pSwapchains = &swapchain_,
        .pImageIndices = &image_index,
    };
    present_result = vkQueuePresentKHR(present_queue_, &present_info);
  }
  const VulkanSwapchainRecoveryPlan present_plan =
      vulkan_plan_swapchain_recovery(
          VulkanSwapchainOperation::present, present_result);
  if (present_result == VK_ERROR_DEVICE_LOST)
    return recover_after_device_loss(present_result, "vkQueuePresentKHR lost device");
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
        ErrorCode::frame_retry_required,
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
  last_frame_diagnostic_snapshot_.backdrop_blur = backdrop_blur_diagnostics();
  vulkan_report_retained_images(retained_image_uploads_, last_frame_diagnostic_snapshot_);
  has_frame_diagnostic_snapshot_ = true;

  return {};
}

} // namespace cgpui
