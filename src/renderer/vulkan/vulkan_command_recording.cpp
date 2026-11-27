#include "vulkan_command_recording_internal.hpp"
#include "vulkan_frame_draw_recording_internal.hpp"
#include "vulkan_image_submission_diagnostics_internal.hpp"

namespace cgpui {

Result<VulkanFrameCommandRecordingResult> record_vulkan_frame_command_buffer(
    const VulkanFrameRecordingInput& input,
    VulkanFrameCommandReuseState& command_reuse_state) {
  const auto& target = input.target;
  const auto& geometry = input.geometry;
  const auto& text = input.text;
  const auto& images = input.images;
  const auto& effects = input.effects;
  const VulkanBoxShadowDrawResources& box_shadow = effects.box_shadow;
  if (auto result = vulkan_validate_glyph_atlas_draw_bindings(
          text.glyph_atlas_draw_bindings,
          text.glyph_atlas_draw_quads,
          text.glyph_atlas);
      !result) {
    return std::unexpected(result.error());
  }
  auto text_draw_commands = vulkan_plan_text_draw_commands(
      text.glyph_atlas_draw_bindings,
      text.pipeline,
      text.vertex_buffer);
  if (!text_draw_commands) {
    return std::unexpected(text_draw_commands.error());
  }
  auto image_draw_commands = vulkan_plan_image_draw_commands(
      images.draws,
      images.textures,
      images.pipeline,
      images.vertex_buffer,
      &images.uploads);
  if (!image_draw_commands) {
    return std::unexpected(image_draw_commands.error());
  }
  if (auto result = vulkan_validate_rounded_rect_draw_resources(
          geometry.pipeline, geometry.solid_rect_buffers);
      !result) {
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_validate_rounded_rect_draw_resources(
          geometry.pipeline, geometry.rounded_rect_buffers);
      !result) {
    return std::unexpected(result.error());
  }
  const VulkanRoundedRectBufferResources no_box_shadows;
  const VulkanRoundedRectPipelineResources no_box_shadow_pipeline;
  const auto& box_shadow_buffers =
      box_shadow.buffers != nullptr ? *box_shadow.buffers : no_box_shadows;
  const auto& box_shadow_pipeline = box_shadow.pipeline != nullptr
                                        ? *box_shadow.pipeline
                                        : no_box_shadow_pipeline;
  if (auto result = vulkan_validate_rounded_rect_draw_resources(
          box_shadow_pipeline, box_shadow_buffers);
      !result) {
    return std::unexpected(result.error());
  }

  const bool has_pending_uploads =
      !text.uploads.uploads.empty() || !images.uploads.uploads.empty();
  const VulkanFrameCommandSignatureView signature{
      .render_pass = target.render_pass,
      .framebuffer = target.framebuffer,
      .extent = target.extent,
      .clear_color = target.clear_color,
      .rounded_rect_pipeline_layout = geometry.pipeline.layout,
      .rounded_rect_pipeline = geometry.pipeline.pipeline,
      .text_pipeline_layout = text.pipeline.layout,
      .text_pipeline = text.pipeline.pipeline,
      .image_pipeline_layout = images.pipeline.layout,
      .image_pipeline = images.pipeline.pipeline,
      .box_shadow_pipeline = box_shadow_pipeline.pipeline,
      .solid_vertex_buffer = geometry.solid_rect_buffers.vertices.buffer,
      .solid_index_buffer = geometry.solid_rect_buffers.indices.buffer,
      .rounded_vertex_buffer = geometry.rounded_rect_buffers.vertices.buffer,
      .rounded_index_buffer = geometry.rounded_rect_buffers.indices.buffer,
      .text_vertex_buffer = text.vertex_buffer.vertices.buffer,
      .image_vertex_buffer = images.vertex_buffer.vertices.buffer,
      .box_shadow_vertex_buffer = box_shadow_buffers.vertices.buffer,
      .box_shadow_index_buffer = box_shadow_buffers.indices.buffer,
      .solid_draws = geometry.solid_rect_buffers.draws,
      .rounded_draws = geometry.rounded_rect_buffers.draws,
      .text_commands = *text_draw_commands,
      .image_commands = *image_draw_commands,
      .box_shadow_draws = box_shadow_buffers.draws,
      .draw_order = input.draw_order,
  };
  // Backdrop frames sample their own offscreen output and are never reused.
  const bool single_use =
      input.pixel_capture.enabled() || effects.backdrop_blur.active();
  const VulkanFrameCommandReusePlan reuse_plan =
      vulkan_plan_frame_command_reuse(
          command_reuse_state, signature, has_pending_uploads,
          single_use);
  if (reuse_plan.action == VulkanFrameCommandReuseAction::reuse) {
    vulkan_commit_frame_command_reuse(command_reuse_state);
    return VulkanFrameCommandRecordingResult{
        .action = reuse_plan.action,
        .reason = reuse_plan.reason,
        .submitted_image_indices = vulkan_submitted_image_indices(
            target.extent, input.draw_order, *image_draw_commands),
    };
  }
  vulkan_invalidate_frame_command_reuse(command_reuse_state);

  if (auto result = require_vk_success(
          vkResetCommandBuffer(target.command_buffer, 0),
          "vkResetCommandBuffer failed");
      !result) {
    return std::unexpected(result.error());
  }

  const VkCommandBufferBeginInfo begin_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = 0,
  };
  if (auto result = require_vk_success(
          vkBeginCommandBuffer(target.command_buffer, &begin_info),
          "vkBeginCommandBuffer failed");
      !result) {
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_record_glyph_atlas_uploads(
          target.command_buffer,
          text.glyph_atlas,
          text.uploads);
      !result) {
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_record_image_texture_uploads(
          target.command_buffer,
          images.textures,
          images.uploads);
      !result) {
    return std::unexpected(result.error());
  }
  const auto record_draws =
      [&](std::span<const VulkanFrameDrawOrderEntry> segment) {
        vulkan_record_frame_draws(
            target.command_buffer, target.extent, geometry.pipeline,
            geometry.solid_rect_buffers, geometry.rounded_rect_buffers, text.pipeline,
            text.vertex_buffer, *text_draw_commands, images.pipeline,
            images.vertex_buffer, *image_draw_commands, segment, box_shadow);
      };
  if (effects.backdrop_blur.active()) {
    (void)vulkan_record_backdrop_blur_frame(
        target.command_buffer, effects.backdrop_blur,
        {.render_pass = target.render_pass, .framebuffer = target.framebuffer,
         .extent = target.extent, .clear_color = target.clear_color},
        input.draw_order, record_draws);
  } else {
    VkClearValue clear_value{};
    clear_value.color = VkClearColorValue{{
        target.clear_color.r, target.clear_color.g,
        target.clear_color.b, target.clear_color.a}};
    const VkRenderPassBeginInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = target.render_pass,
        .framebuffer = target.framebuffer,
        .renderArea = {.offset = {.x = 0, .y = 0}, .extent = target.extent},
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };
    vkCmdBeginRenderPass(
        target.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    record_draws(input.draw_order);
    vkCmdEndRenderPass(target.command_buffer);
  }
  vulkan_record_frame_pixel_capture(target.command_buffer, input.pixel_capture);
  if (auto result = require_vk_success(
          vkEndCommandBuffer(target.command_buffer),
          "vkEndCommandBuffer failed");
      !result) {
    return std::unexpected(result.error());
  }
  vulkan_commit_frame_command_recording(
      command_reuse_state, signature, has_pending_uploads,
      single_use);
  return VulkanFrameCommandRecordingResult{
      .action = VulkanFrameCommandReuseAction::record,
      .reason = reuse_plan.reason,
      .submitted_image_indices = vulkan_submitted_image_indices(
          target.extent, input.draw_order, *image_draw_commands),
  };
}

} // namespace cgpui
