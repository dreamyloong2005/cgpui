#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> record_vulkan_frame_command_buffer(
    VkCommandBuffer command_buffer,
    VkRenderPass render_pass,
    VkFramebuffer framebuffer,
    VkExtent2D extent,
    const VulkanRoundedRectPipelineResources& rounded_rect_pipeline_resources,
    const VulkanRoundedRectBufferResources& rounded_rect_buffers,
    const VulkanTextPipelineResources& text_pipeline_resources,
    const VulkanTextVertexBufferResources& text_vertex_buffer,
    Color color,
    std::span<const SolidRect> rects,
    const VulkanGlyphAtlasResources& glyph_atlas_resources,
    std::span<const TexturedGlyphQuad> glyph_atlas_draw_quads,
    std::span<const VulkanGlyphAtlasDrawBinding> glyph_atlas_draw_bindings,
    const VulkanGlyphAtlasUploadResources& glyph_atlas_uploads) {
  if (auto result = require_vk_success(
          vkResetCommandBuffer(command_buffer, 0),
          "vkResetCommandBuffer failed");
      !result) {
    return result;
  }

  const VkCommandBufferBeginInfo begin_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  if (auto result = require_vk_success(
          vkBeginCommandBuffer(command_buffer, &begin_info),
          "vkBeginCommandBuffer failed");
      !result) {
    return result;
  }
  if (auto result = vulkan_record_glyph_atlas_uploads(
          command_buffer,
          glyph_atlas_resources,
          glyph_atlas_uploads);
      !result) {
    return result;
  }
  if (auto result = vulkan_validate_glyph_atlas_draw_bindings(
          glyph_atlas_draw_bindings,
          glyph_atlas_draw_quads,
          glyph_atlas_resources);
      !result) {
    return result;
  }
  auto text_draw_commands = vulkan_plan_text_draw_commands(
      glyph_atlas_draw_bindings,
      text_pipeline_resources,
      text_vertex_buffer);
  if (!text_draw_commands) {
    return std::unexpected(text_draw_commands.error());
  }
  if (auto result = vulkan_validate_rounded_rect_draw_resources(
          rounded_rect_pipeline_resources, rounded_rect_buffers);
      !result) {
    return result;
  }
  VkClearValue clear_value{};
  clear_value.color = VkClearColorValue{{color.r, color.g, color.b, color.a}};
  const VkRenderPassBeginInfo render_pass_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = render_pass,
      .framebuffer = framebuffer,
      .renderArea =
          VkRect2D{
              .offset = VkOffset2D{.x = 0, .y = 0},
              .extent = extent,
          },
      .clearValueCount = 1,
      .pClearValues = &clear_value,
  };
  vkCmdBeginRenderPass(
      command_buffer,
      &render_pass_info,
      VK_SUBPASS_CONTENTS_INLINE);

  vulkan_record_solid_rects(command_buffer, extent, rects);
  vulkan_record_rounded_rect_draws(
      command_buffer,
      extent,
      rounded_rect_pipeline_resources,
      rounded_rect_buffers);
  vulkan_record_text_draws(
      command_buffer,
      extent,
      text_pipeline_resources,
      text_vertex_buffer,
      *text_draw_commands);
  vkCmdEndRenderPass(command_buffer);

  return require_vk_success(
      vkEndCommandBuffer(command_buffer),
      "vkEndCommandBuffer failed");
}

} // namespace cgpui
