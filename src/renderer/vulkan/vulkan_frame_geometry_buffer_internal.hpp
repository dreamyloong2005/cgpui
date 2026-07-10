#pragma once

#include "vulkan_platform_internal.hpp"

namespace cgpui {

enum class VulkanFrameGeometryBufferAction {
  invalid,
  retain_empty,
  reuse,
  replace,
};

struct VulkanFrameGeometryBufferResources {
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  std::size_t byte_capacity = 0;
  VkBufferUsageFlags usage = 0;

  [[nodiscard]] bool allocated() const {
    return buffer != VK_NULL_HANDLE && memory != VK_NULL_HANDLE &&
           byte_capacity != 0 && usage != 0;
  }
};

struct VulkanFrameGeometryBufferPlan {
  VulkanFrameGeometryBufferAction action =
      VulkanFrameGeometryBufferAction::invalid;
  std::size_t required_byte_size = 0;
  std::size_t allocation_byte_size = 0;
  VkBufferUsageFlags usage = 0;

  [[nodiscard]] bool valid() const {
    return action != VulkanFrameGeometryBufferAction::invalid &&
           allocation_byte_size >= required_byte_size;
  }
};

[[nodiscard]] VulkanFrameGeometryBufferPlan
vulkan_plan_frame_geometry_buffer(
    const VulkanFrameGeometryBufferResources& resources,
    std::size_t required_byte_size,
    VkBufferUsageFlags usage);
[[nodiscard]] VkBufferCreateInfo vulkan_frame_geometry_buffer_create_info(
    std::size_t byte_size,
    VkBufferUsageFlags usage);
Result<void> vulkan_upload_frame_geometry_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const void* data,
    std::size_t byte_size,
    VkBufferUsageFlags usage,
    VulkanFrameGeometryBufferResources& resources);
void vulkan_destroy_frame_geometry_buffer(
    VkDevice device,
    VulkanFrameGeometryBufferResources& resources);

} // namespace cgpui
