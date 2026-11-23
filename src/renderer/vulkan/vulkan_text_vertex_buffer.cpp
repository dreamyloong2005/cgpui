#include "vulkan_text_vertex_buffer_internal.hpp"
#include "cgpui/renderer/planning/text_geometry.hpp"

namespace cgpui {

std::vector<VulkanTextVertex> vulkan_build_text_vertices(
    std::span<const TexturedGlyphQuad> quads,
    VulkanTextPositioningPolicy positioning_policy) {
  return build_text_vertices(quads, positioning_policy);
}

Result<void> vulkan_upload_text_vertex_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const TexturedGlyphQuad> quads,
    VulkanTextPositioningPolicy positioning_policy,
    VulkanTextVertexBufferResources& resources) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "text vertex upload requires Vulkan physical and logical devices"));
  }
  const std::vector<VulkanTextVertex> vertices =
      vulkan_build_text_vertices(quads, positioning_policy);
  const std::size_t byte_size = vertices.size() * sizeof(VulkanTextVertex);
  if (auto result = vulkan_upload_frame_geometry_buffer(
      physical_device,
      device,
      vertices.data(),
      byte_size,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      resources.vertices);
      !result) {
    return result;
  }
  resources.positioning_policy = positioning_policy;
  resources.vertex_count = vertices.size();
  resources.byte_size = byte_size;
  return {};
}

void vulkan_destroy_text_vertex_buffer(
    VkDevice device,
    VulkanTextVertexBufferResources& resources) {
  if (device == VK_NULL_HANDLE) {
    return;
  }
  vulkan_destroy_frame_geometry_buffer(device, resources.vertices);
  resources = {};
}

} // namespace cgpui
