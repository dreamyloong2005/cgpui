#pragma once

#include "vulkan_glyph_atlas_draw_bindings_internal.hpp"
#include "vulkan_text_pipeline_resources_internal.hpp"
#include "vulkan_text_vertex_buffer_internal.hpp"

namespace cgpui {

struct VulkanTextDrawCommand {
  VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
  std::uint32_t first_vertex = 0;
  std::uint32_t vertex_count = 0;
};

[[nodiscard]] Result<std::vector<VulkanTextDrawCommand>>
vulkan_plan_text_draw_commands(
    std::span<const VulkanGlyphAtlasDrawBinding> bindings,
    const VulkanTextPipelineResources& pipeline_resources,
    const VulkanTextVertexBufferResources& vertex_buffer);
void vulkan_record_text_draws(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanTextPipelineResources& pipeline_resources,
    const VulkanTextVertexBufferResources& vertex_buffer,
    std::span<const VulkanTextDrawCommand> commands);

} // namespace cgpui
