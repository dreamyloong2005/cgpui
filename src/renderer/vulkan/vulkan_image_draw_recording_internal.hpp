#pragma once

#include "vulkan_image_pipeline_resources_internal.hpp"
#include "vulkan_image_texture_descriptors_internal.hpp"
#include "vulkan_image_texture_uploads_internal.hpp"
#include "vulkan_image_vertex_buffer_internal.hpp"

namespace cgpui {

struct VulkanImageDrawCommand {
  std::size_t image_draw_index = 0;
  VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
  std::uint32_t first_vertex = 0;
  std::uint32_t vertex_count = 0;
  std::optional<Rect> clip_rect;
  RendererClipStackRecord clip_stack;
};

[[nodiscard]] Result<std::vector<VulkanImageDrawCommand>>
vulkan_plan_image_draw_commands(
    std::span<const ImageDraw> image_draws,
    const VulkanImageTextureResources& texture_resources,
    const VulkanImagePipelineResources& pipeline_resources,
    const VulkanImageVertexBufferResources& vertex_buffer,
    const VulkanImageTextureUploadResources* upload_resources = nullptr);
void vulkan_bind_image_draw_state(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanImagePipelineResources& pipeline_resources,
    const VulkanImageVertexBufferResources& vertex_buffer,
    bool bind_pipeline = true);
void vulkan_record_image_draw(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanImagePipelineResources& pipeline_resources,
    const VulkanImageDrawCommand& command);
void vulkan_record_image_draws(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanImagePipelineResources& pipeline_resources,
    const VulkanImageVertexBufferResources& vertex_buffer,
    std::span<const VulkanImageDrawCommand> commands);

} // namespace cgpui
