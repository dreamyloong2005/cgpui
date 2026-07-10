#include "vulkan_image_draw_recording_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"

#include <algorithm>
#include <limits>

namespace cgpui {
namespace {

bool image_texture_shader_readable_for_frame(
    const VulkanImageTextureResource& texture,
    const VulkanImageTextureUploadResources* upload_resources) {
  if (texture.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    return true;
  }
  return upload_resources != nullptr &&
         std::ranges::any_of(
             upload_resources->uploads,
             [&texture](const VulkanImageTextureStagingUpload& upload) {
               return upload.asset_id == texture.descriptor.id;
             });
}

} // namespace

Result<std::vector<VulkanImageDrawCommand>>
vulkan_plan_image_draw_commands(
    std::span<const ImageDraw> image_draws,
    const VulkanImageTextureResources& texture_resources,
    const VulkanImagePipelineResources& pipeline_resources,
    const VulkanImageVertexBufferResources& vertex_buffer,
    const VulkanImageTextureUploadResources* upload_resources) {
  std::vector<VulkanImageDrawCommand> commands;
  if (image_draws.empty()) {
    return commands;
  }
  if (!pipeline_resources.ready() || !vertex_buffer.ready()) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image draws require pipeline and vertex-buffer resources"));
  }

  commands.reserve(image_draws.size());
  constexpr std::size_t max_draw_vertices =
      std::numeric_limits<std::uint32_t>::max();
  for (std::size_t index = 0; index < image_draws.size(); ++index) {
    const ImageDraw& draw = image_draws[index];
    const VulkanImageTextureResource* texture =
        vulkan_find_image_texture_resource(texture_resources, draw.asset.id);
    if (texture != nullptr &&
        !image_texture_shader_readable_for_frame(
            *texture, upload_resources)) {
      continue;
    }
    const VkDescriptorSet descriptor_set =
        texture == nullptr
            ? VK_NULL_HANDLE
            : vulkan_image_texture_descriptor_set(*texture, draw.sampling);
    if (descriptor_set == VK_NULL_HANDLE ||
        index > max_draw_vertices / vulkan_image_vertices_per_quad) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "image draw texture binding is unavailable"));
    }
    const std::size_t first_vertex =
        index * vulkan_image_vertices_per_quad;
    if (first_vertex > vertex_buffer.vertex_count ||
        vulkan_image_vertices_per_quad >
            vertex_buffer.vertex_count - first_vertex) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "image draw vertex range exceeds the uploaded buffer"));
    }
    commands.push_back(VulkanImageDrawCommand{
        .image_draw_index = index,
        .descriptor_set = descriptor_set,
        .first_vertex = static_cast<std::uint32_t>(first_vertex),
        .vertex_count =
            static_cast<std::uint32_t>(vulkan_image_vertices_per_quad),
        .clip_rect = draw.clip_rect,
        .clip_stack = draw.clip_stack,
    });
  }
  return commands;
}

void vulkan_bind_image_draw_state(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanImagePipelineResources& pipeline_resources,
    const VulkanImageVertexBufferResources& vertex_buffer) {
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
  const VulkanImagePushConstants push_constants{
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
}

void vulkan_record_image_draw(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanImagePipelineResources& pipeline_resources,
    const VulkanImageDrawCommand& command) {
  const VulkanClipScissorResolution clip = vulkan_resolve_clip_stack_scissor(
      extent, command.clip_rect, command.clip_stack);
  if (!clip.visible) {
    return;
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
      command_buffer, command.vertex_count, 1, command.first_vertex, 0);
}

void vulkan_record_image_draws(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanImagePipelineResources& pipeline_resources,
    const VulkanImageVertexBufferResources& vertex_buffer,
    std::span<const VulkanImageDrawCommand> commands) {
  if (commands.empty()) {
    return;
  }
  vulkan_bind_image_draw_state(
      command_buffer, extent, pipeline_resources, vertex_buffer);
  for (const VulkanImageDrawCommand& command : commands) {
    vulkan_record_image_draw(
        command_buffer, extent, pipeline_resources, command);
  }
}

} // namespace cgpui
