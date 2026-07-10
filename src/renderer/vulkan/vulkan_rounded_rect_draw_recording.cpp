#include "vulkan_rounded_rect_draw_recording_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"

#include <limits>

namespace cgpui {

Result<void> vulkan_validate_rounded_rect_draw_resources(
    const VulkanRoundedRectPipelineResources& pipeline_resources,
    const VulkanRoundedRectBufferResources& buffer_resources) {
  if (buffer_resources.draws.empty()) {
    return {};
  }
  if (!pipeline_resources.ready() || !buffer_resources.ready()) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "rounded rectangle draws require pipeline and buffer resources"));
  }

  constexpr std::size_t max_draw_indices =
      std::numeric_limits<std::uint32_t>::max();
  for (const VulkanRoundedRectDrawRange& draw : buffer_resources.draws) {
    if (draw.vertex_count == 0 || draw.index_count == 0 ||
        draw.first_vertex > buffer_resources.vertex_count ||
        draw.vertex_count >
            buffer_resources.vertex_count - draw.first_vertex ||
        draw.first_index > buffer_resources.index_count ||
        draw.index_count > buffer_resources.index_count - draw.first_index ||
        draw.first_index > max_draw_indices ||
        draw.index_count > max_draw_indices) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "rounded rectangle draw range exceeds uploaded buffers"));
    }
  }
  return {};
}

void vulkan_record_rounded_rect_draws(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanRoundedRectPipelineResources& pipeline_resources,
    const VulkanRoundedRectBufferResources& buffer_resources) {
  if (buffer_resources.draws.empty()) {
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

  const VkBuffer vertex_buffer = buffer_resources.vertex_buffer;
  constexpr VkDeviceSize buffer_offset = 0;
  vkCmdBindVertexBuffers(
      command_buffer, 0, 1, &vertex_buffer, &buffer_offset);
  vkCmdBindIndexBuffer(
      command_buffer,
      buffer_resources.index_buffer,
      0,
      VK_INDEX_TYPE_UINT32);
  const VulkanRoundedRectPushConstants push_constants{
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

  for (const VulkanRoundedRectDrawRange& draw : buffer_resources.draws) {
    const VulkanClipScissorResolution clip =
        vulkan_resolve_clip_stack_scissor(
            extent, draw.clip_rect, RendererClipStackRecord{});
    if (!clip.visible) {
      continue;
    }
    vkCmdSetScissor(command_buffer, 0, 1, &clip.scissor);
    vkCmdDrawIndexed(
        command_buffer,
        static_cast<std::uint32_t>(draw.index_count),
        1,
        static_cast<std::uint32_t>(draw.first_index),
        0,
        0);
  }
}

} // namespace cgpui
