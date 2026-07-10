#pragma once

#include "vulkan_frame_geometry_buffer_internal.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"

namespace cgpui {

struct VulkanRoundedRectBufferResources {
  VulkanFrameGeometryBufferResources vertices;
  VulkanFrameGeometryBufferResources indices;
  std::size_t vertex_count = 0;
  std::size_t index_count = 0;
  std::size_t vertex_byte_size = 0;
  std::size_t index_byte_size = 0;
  std::vector<VulkanRoundedRectDrawRange> draws;

  [[nodiscard]] bool ready() const {
    return vertices.allocated() && indices.allocated() && vertex_count != 0 &&
           index_count != 0 &&
           vertex_byte_size != 0 && index_byte_size != 0 && !draws.empty();
  }
};

Result<void> vulkan_upload_rounded_rect_buffers(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const RoundedRectDraw> rounded_rects,
    VulkanRoundedRectBufferResources& resources);
Result<void> vulkan_upload_solid_rect_buffers(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const SolidRect> rects,
    VulkanRoundedRectBufferResources& resources);
void vulkan_destroy_rounded_rect_buffers(
    VkDevice device,
    VulkanRoundedRectBufferResources& resources);

} // namespace cgpui
