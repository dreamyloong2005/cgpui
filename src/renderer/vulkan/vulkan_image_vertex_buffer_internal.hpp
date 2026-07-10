#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "vulkan_frame_geometry_buffer_internal.hpp"
#include "vulkan_image_pipeline_internal.hpp"

namespace cgpui {

inline constexpr std::size_t vulkan_image_vertices_per_quad = 6;

struct VulkanImageVertexBufferResources {
  VulkanFrameGeometryBufferResources vertices;
  std::size_t vertex_count = 0;
  std::size_t byte_size = 0;

  [[nodiscard]] bool ready() const {
    return vertices.allocated() && vertex_count != 0 && byte_size != 0;
  }
};

[[nodiscard]] std::vector<VulkanImageVertex> vulkan_build_image_vertices(
    std::span<const ImageDraw> image_draws);
Result<void> vulkan_upload_image_vertex_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const ImageDraw> image_draws,
    VulkanImageVertexBufferResources& resources);
void vulkan_destroy_image_vertex_buffer(
    VkDevice device,
    VulkanImageVertexBufferResources& resources);

} // namespace cgpui
