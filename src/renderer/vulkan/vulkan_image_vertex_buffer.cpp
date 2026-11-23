#include "vulkan_image_vertex_buffer_internal.hpp"
#include "cgpui/renderer/planning/image_geometry.hpp"

namespace cgpui {

std::vector<VulkanImageVertex> vulkan_build_image_vertices(
    std::span<const ImageDraw> image_draws) {
  return build_image_vertices(image_draws);
}

Result<void> vulkan_upload_image_vertex_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const ImageDraw> image_draws,
    VulkanImageVertexBufferResources& resources) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image vertex upload requires Vulkan physical and logical devices"));
  }
  const std::vector<VulkanImageVertex> vertices =
      vulkan_build_image_vertices(image_draws);
  const std::size_t byte_size = vertices.size() * sizeof(VulkanImageVertex);
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
  resources.vertex_count = vertices.size();
  resources.byte_size = byte_size;
  return {};
}

void vulkan_destroy_image_vertex_buffer(
    VkDevice device,
    VulkanImageVertexBufferResources& resources) {
  if (device == VK_NULL_HANDLE) {
    return;
  }
  vulkan_destroy_frame_geometry_buffer(device, resources.vertices);
  resources = {};
}

} // namespace cgpui
