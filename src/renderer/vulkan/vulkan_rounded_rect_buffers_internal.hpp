#pragma once

#include "vulkan_platform_internal.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"

namespace cgpui {

struct VulkanRoundedRectBufferResources {
  VkBuffer vertex_buffer = VK_NULL_HANDLE;
  VkDeviceMemory vertex_memory = VK_NULL_HANDLE;
  VkBuffer index_buffer = VK_NULL_HANDLE;
  VkDeviceMemory index_memory = VK_NULL_HANDLE;
  std::size_t vertex_count = 0;
  std::size_t index_count = 0;
  std::size_t vertex_byte_size = 0;
  std::size_t index_byte_size = 0;
  std::vector<VulkanRoundedRectDrawRange> draws;

  [[nodiscard]] bool ready() const {
    return vertex_buffer != VK_NULL_HANDLE &&
           vertex_memory != VK_NULL_HANDLE &&
           index_buffer != VK_NULL_HANDLE && index_memory != VK_NULL_HANDLE &&
           vertex_count != 0 && index_count != 0 &&
           vertex_byte_size != 0 && index_byte_size != 0 && !draws.empty();
  }
};

[[nodiscard]] VkBufferCreateInfo vulkan_rounded_rect_buffer_create_info(
    std::size_t byte_size,
    VkBufferUsageFlags usage);
Result<void> vulkan_upload_rounded_rect_buffers(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const RoundedRectDraw> rounded_rects,
    VulkanRoundedRectBufferResources& resources);
void vulkan_destroy_rounded_rect_buffers(
    VkDevice device,
    VulkanRoundedRectBufferResources& resources);

} // namespace cgpui
