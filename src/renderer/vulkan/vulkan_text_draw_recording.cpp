#include "vulkan_text_draw_recording_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"

#include <limits>

namespace cgpui {

Result<std::vector<VulkanTextDrawCommand>> vulkan_plan_text_draw_commands(
    std::span<const VulkanGlyphAtlasDrawBinding> bindings,
    const VulkanTextPipelineResources& pipeline_resources,
    const VulkanTextVertexBufferResources& vertex_buffer) {
  std::vector<VulkanTextDrawCommand> commands;
  if (bindings.empty()) {
    return commands;
  }
  if (!pipeline_resources.ready() || !vertex_buffer.ready()) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "text draws require pipeline and vertex-buffer resources"));
  }

  commands.reserve(bindings.size());
  constexpr std::size_t max_draw_vertices =
      std::numeric_limits<std::uint32_t>::max();
  for (const VulkanGlyphAtlasDrawBinding& binding : bindings) {
    if (binding.descriptor_set == VK_NULL_HANDLE ||
        binding.glyph_quad_count == 0 ||
        binding.first_quad_index >
            max_draw_vertices / vulkan_text_vertices_per_quad ||
        binding.glyph_quad_count >
            max_draw_vertices / vulkan_text_vertices_per_quad) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "text draw page range is invalid"));
    }
    const std::size_t first_vertex =
        binding.first_quad_index * vulkan_text_vertices_per_quad;
    const std::size_t vertex_count =
        binding.glyph_quad_count * vulkan_text_vertices_per_quad;
    if (first_vertex > vertex_buffer.vertex_count ||
        vertex_count > vertex_buffer.vertex_count - first_vertex) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "text draw vertex range exceeds the uploaded buffer"));
    }
    commands.push_back(VulkanTextDrawCommand{
        .descriptor_set = binding.descriptor_set,
        .first_vertex = static_cast<std::uint32_t>(first_vertex),
        .vertex_count = static_cast<std::uint32_t>(vertex_count),
        .clip_rect = binding.clip_rect,
    });
  }
  return commands;
}

void vulkan_record_text_draws(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanTextPipelineResources& pipeline_resources,
    const VulkanTextVertexBufferResources& vertex_buffer,
    std::span<const VulkanTextDrawCommand> commands) {
  if (commands.empty()) {
    return;
  }

  vkCmdBindPipeline(
      command_buffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipeline_resources.pipeline);
  const VkViewport viewport{
      .x = 0.0F,
      .y = 0.0F,
      .width = static_cast<float>(extent.width),
      .height = static_cast<float>(extent.height),
      .minDepth = 0.0F,
      .maxDepth = 1.0F,
  };
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  const VkBuffer buffer = vertex_buffer.buffer;
  constexpr VkDeviceSize buffer_offset = 0;
  vkCmdBindVertexBuffers(command_buffer, 0, 1, &buffer, &buffer_offset);
  const VulkanTextPushConstants push_constants{
      .framebuffer_size = {
          static_cast<float>(extent.width),
          static_cast<float>(extent.height),
      },
  };
  vkCmdPushConstants(
      command_buffer,
      pipeline_resources.layout,
      VK_SHADER_STAGE_VERTEX_BIT,
      0,
      sizeof(push_constants),
      &push_constants);

  for (const VulkanTextDrawCommand& command : commands) {
    const VulkanClipScissorResolution clip =
        vulkan_resolve_clip_stack_scissor(
            extent, command.clip_rect, RendererClipStackRecord{});
    if (!clip.visible) {
      continue;
    }
    vkCmdSetScissor(command_buffer, 0, 1, &clip.scissor);
    vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_resources.layout,
        0,
        1,
        &command.descriptor_set,
        0,
        nullptr);
    vkCmdDraw(
        command_buffer,
        command.vertex_count,
        1,
        command.first_vertex,
        0);
  }
}

} // namespace cgpui
