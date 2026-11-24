#include "vulkan_internal.hpp"
#include "vulkan_image_submission_diagnostics_internal.hpp"

namespace cgpui {

Result<VulkanFrameCommandRecordingResult> record_vulkan_frame_command_buffer(
    VkCommandBuffer command_buffer,
    VkRenderPass render_pass,
    VkFramebuffer framebuffer,
    VkExtent2D extent,
    const VulkanRoundedRectPipelineResources& rounded_rect_pipeline_resources,
    const VulkanRoundedRectBufferResources& solid_rect_buffers,
    const VulkanRoundedRectBufferResources& rounded_rect_buffers,
    const VulkanTextPipelineResources& text_pipeline_resources,
    const VulkanTextVertexBufferResources& text_vertex_buffer,
    const VulkanImagePipelineResources& image_pipeline_resources,
    const VulkanImageVertexBufferResources& image_vertex_buffer,
    std::span<const ImageDraw> image_draws,
    Color color,
    std::span<const VulkanFrameDrawOrderEntry> draw_order,
    const VulkanGlyphAtlasResources& glyph_atlas_resources,
    std::span<const TexturedGlyphQuad> glyph_atlas_draw_quads,
    std::span<const VulkanGlyphAtlasDrawBinding> glyph_atlas_draw_bindings,
    const VulkanGlyphAtlasUploadResources& glyph_atlas_uploads,
    const VulkanImageTextureResources& image_texture_resources,
    const VulkanImageTextureUploadResources& image_texture_uploads,
    const VulkanFramePixelCaptureCommand& pixel_capture,
    VulkanFrameCommandReuseState& command_reuse_state,
    const VulkanFrameEffects& effects) {
  const VulkanBoxShadowDrawResources& box_shadow = effects.box_shadow;
  if (auto result = vulkan_validate_glyph_atlas_draw_bindings(
          glyph_atlas_draw_bindings,
          glyph_atlas_draw_quads,
          glyph_atlas_resources);
      !result) {
    return std::unexpected(result.error());
  }
  auto text_draw_commands = vulkan_plan_text_draw_commands(
      glyph_atlas_draw_bindings,
      text_pipeline_resources,
      text_vertex_buffer);
  if (!text_draw_commands) {
    return std::unexpected(text_draw_commands.error());
  }
  auto image_draw_commands = vulkan_plan_image_draw_commands(
      image_draws,
      image_texture_resources,
      image_pipeline_resources,
      image_vertex_buffer,
      &image_texture_uploads);
  if (!image_draw_commands) {
    return std::unexpected(image_draw_commands.error());
  }
  if (auto result = vulkan_validate_rounded_rect_draw_resources(
          rounded_rect_pipeline_resources, solid_rect_buffers);
      !result) {
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_validate_rounded_rect_draw_resources(
          rounded_rect_pipeline_resources, rounded_rect_buffers);
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

  const bool has_pending_uploads = !glyph_atlas_uploads.uploads.empty() ||
                                   !image_texture_uploads.uploads.empty();
  const VulkanFrameCommandSignatureView signature{
      .render_pass = render_pass,
      .framebuffer = framebuffer,
      .extent = extent,
      .clear_color = color,
      .rounded_rect_pipeline_layout = rounded_rect_pipeline_resources.layout,
      .rounded_rect_pipeline = rounded_rect_pipeline_resources.pipeline,
      .text_pipeline_layout = text_pipeline_resources.layout,
      .text_pipeline = text_pipeline_resources.pipeline,
      .image_pipeline_layout = image_pipeline_resources.layout,
      .image_pipeline = image_pipeline_resources.pipeline,
      .box_shadow_pipeline = box_shadow_pipeline.pipeline,
      .solid_vertex_buffer = solid_rect_buffers.vertices.buffer,
      .solid_index_buffer = solid_rect_buffers.indices.buffer,
      .rounded_vertex_buffer = rounded_rect_buffers.vertices.buffer,
      .rounded_index_buffer = rounded_rect_buffers.indices.buffer,
      .text_vertex_buffer = text_vertex_buffer.vertices.buffer,
      .image_vertex_buffer = image_vertex_buffer.vertices.buffer,
      .box_shadow_vertex_buffer = box_shadow_buffers.vertices.buffer,
      .box_shadow_index_buffer = box_shadow_buffers.indices.buffer,
      .solid_draws = solid_rect_buffers.draws,
      .rounded_draws = rounded_rect_buffers.draws,
      .text_commands = *text_draw_commands,
      .image_commands = *image_draw_commands,
      .box_shadow_draws = box_shadow_buffers.draws,
      .draw_order = draw_order,
  };
  // Backdrop frames sample their own offscreen output and are never reused.
  const bool single_use =
      pixel_capture.enabled() || effects.backdrop_blur.active();
  const VulkanFrameCommandReusePlan reuse_plan =
      vulkan_plan_frame_command_reuse(
          command_reuse_state, signature, has_pending_uploads,
          single_use);
  if (reuse_plan.action == VulkanFrameCommandReuseAction::reuse) {
    vulkan_commit_frame_command_reuse(command_reuse_state);
    return VulkanFrameCommandRecordingResult{
        .action = reuse_plan.action,
        .reason = reuse_plan.reason,
        .submitted_image_indices = vulkan_submitted_image_indices(extent, draw_order, *image_draw_commands),
    };
  }
  vulkan_invalidate_frame_command_reuse(command_reuse_state);

  if (auto result = require_vk_success(
          vkResetCommandBuffer(command_buffer, 0),
          "vkResetCommandBuffer failed");
      !result) {
    return std::unexpected(result.error());
  }

  const VkCommandBufferBeginInfo begin_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = 0,
  };
  if (auto result = require_vk_success(
          vkBeginCommandBuffer(command_buffer, &begin_info),
          "vkBeginCommandBuffer failed");
      !result) {
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_record_glyph_atlas_uploads(
          command_buffer,
          glyph_atlas_resources,
          glyph_atlas_uploads);
      !result) {
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_record_image_texture_uploads(
          command_buffer,
          image_texture_resources,
          image_texture_uploads);
      !result) {
    return std::unexpected(result.error());
  }
  const auto record_draws =
      [&](std::span<const VulkanFrameDrawOrderEntry> segment) {
        vulkan_record_frame_draws(
            command_buffer, extent, rounded_rect_pipeline_resources,
            solid_rect_buffers, rounded_rect_buffers, text_pipeline_resources,
            text_vertex_buffer, *text_draw_commands, image_pipeline_resources,
            image_vertex_buffer, *image_draw_commands, segment, box_shadow);
      };
  if (effects.backdrop_blur.active()) {
    (void)vulkan_record_backdrop_blur_frame(
        command_buffer, effects.backdrop_blur,
        {.render_pass = render_pass, .framebuffer = framebuffer,
         .extent = extent, .clear_color = color},
        draw_order, record_draws);
  } else {
    VkClearValue clear_value{};
    clear_value.color = VkClearColorValue{{color.r, color.g, color.b, color.a}};
    const VkRenderPassBeginInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = framebuffer,
        .renderArea = {.offset = {.x = 0, .y = 0}, .extent = extent},
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };
    vkCmdBeginRenderPass(
        command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    record_draws(draw_order);
    vkCmdEndRenderPass(command_buffer);
  }
  vulkan_record_frame_pixel_capture(command_buffer, pixel_capture);
  if (auto result = require_vk_success(
          vkEndCommandBuffer(command_buffer),
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
      .submitted_image_indices = vulkan_submitted_image_indices(extent, draw_order, *image_draw_commands),
  };
}

} // namespace cgpui
