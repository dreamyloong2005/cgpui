#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "vulkan_image_pipeline_internal.hpp"

namespace cgpui {

inline constexpr std::size_t vulkan_image_vertices_per_quad = 6;

struct VulkanImageVertexBufferResources {
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  std::size_t vertex_count = 0;
  std::size_t byte_size = 0;

  [[nodiscard]] bool ready() const {
    return buffer != VK_NULL_HANDLE && memory != VK_NULL_HANDLE &&
           vertex_count != 0 && byte_size != 0;
  }
};

[[nodiscard]] std::vector<VulkanImageVertex> vulkan_build_image_vertices(
    std::span<const ImageDraw> image_draws);
[[nodiscard]] VkBufferCreateInfo vulkan_image_vertex_buffer_create_info(
    std::size_t byte_size);
Result<void> vulkan_upload_image_vertex_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const ImageDraw> image_draws,
    VulkanImageVertexBufferResources& resources);
void vulkan_destroy_image_vertex_buffer(
    VkDevice device,
    VulkanImageVertexBufferResources& resources);

} // namespace cgpui
