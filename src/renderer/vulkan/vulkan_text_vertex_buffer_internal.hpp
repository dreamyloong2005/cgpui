#pragma once

#include "cgpui/renderer/glyph_atlas_types.hpp"
#include "vulkan_text_pipeline_internal.hpp"

namespace cgpui {

inline constexpr std::size_t vulkan_text_vertices_per_quad = 6;

struct VulkanTextVertexBufferResources {
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  std::size_t vertex_count = 0;
  std::size_t byte_size = 0;

  [[nodiscard]] bool ready() const {
    return buffer != VK_NULL_HANDLE && memory != VK_NULL_HANDLE &&
           vertex_count != 0 && byte_size != 0;
  }
};

[[nodiscard]] std::vector<VulkanTextVertex> vulkan_build_text_vertices(
    std::span<const TexturedGlyphQuad> quads);
[[nodiscard]] VkBufferCreateInfo vulkan_text_vertex_buffer_create_info(
    std::size_t byte_size);
Result<void> vulkan_upload_text_vertex_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const TexturedGlyphQuad> quads,
    VulkanTextVertexBufferResources& resources);
void vulkan_destroy_text_vertex_buffer(
    VkDevice device,
    VulkanTextVertexBufferResources& resources);

} // namespace cgpui
