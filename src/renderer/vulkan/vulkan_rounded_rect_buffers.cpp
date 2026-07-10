#include "vulkan_rounded_rect_buffers_internal.hpp"

#include "vulkan_device_internal.hpp"
#include "vulkan_solid_rect_geometry_internal.hpp"

#include <cstring>

namespace cgpui {
namespace {

Result<void> upload_host_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const void* data,
    std::size_t byte_size,
    VkBufferUsageFlags usage,
    VkBuffer& buffer,
    VkDeviceMemory& memory) {
  const VkBufferCreateInfo buffer_info =
      vulkan_rounded_rect_buffer_create_info(byte_size, usage);
  if (auto result = require_vk_success(
          vkCreateBuffer(device, &buffer_info, nullptr, &buffer),
          "vkCreateBuffer for rounded rectangle geometry failed");
      !result) {
    return result;
  }
  VkMemoryRequirements requirements{};
  vkGetBufferMemoryRequirements(device, buffer, &requirements);
  auto memory_type = vulkan_find_memory_type(
      physical_device,
      requirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (!memory_type) {
    return std::unexpected(memory_type.error());
  }
  const VkMemoryAllocateInfo allocation_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = requirements.size,
      .memoryTypeIndex = *memory_type,
  };
  if (auto result = require_vk_success(
          vkAllocateMemory(device, &allocation_info, nullptr, &memory),
          "vkAllocateMemory for rounded rectangle geometry failed");
      !result) {
    return result;
  }
  if (auto result = require_vk_success(
          vkBindBufferMemory(device, buffer, memory, 0),
          "vkBindBufferMemory for rounded rectangle geometry failed");
      !result) {
    return result;
  }
  void* mapped = nullptr;
  if (auto result = require_vk_success(
          vkMapMemory(device, memory, 0, byte_size, 0, &mapped),
          "vkMapMemory for rounded rectangle geometry failed");
      !result) {
    return result;
  }
  std::memcpy(mapped, data, byte_size);
  vkUnmapMemory(device, memory);
  return {};
}

Result<void> upload_geometry(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VulkanRoundedRectGeometry geometry,
    VulkanRoundedRectBufferResources& resources) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "rectangle upload requires Vulkan devices"));
  }
  vulkan_destroy_rounded_rect_buffers(device, resources);
  if (geometry.vertices.empty()) {
    return {};
  }
  const std::size_t vertex_byte_size =
      geometry.vertices.size() * sizeof(VulkanRoundedRectVertex);
  const std::size_t index_byte_size =
      geometry.indices.size() * sizeof(std::uint32_t);
  if (auto result = upload_host_buffer(
          physical_device,
          device,
          geometry.vertices.data(),
          vertex_byte_size,
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
          resources.vertex_buffer,
          resources.vertex_memory);
      !result) {
    vulkan_destroy_rounded_rect_buffers(device, resources);
    return result;
  }
  if (auto result = upload_host_buffer(
          physical_device,
          device,
          geometry.indices.data(),
          index_byte_size,
          VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
          resources.index_buffer,
          resources.index_memory);
      !result) {
    vulkan_destroy_rounded_rect_buffers(device, resources);
    return result;
  }
  resources.vertex_count = geometry.vertices.size();
  resources.index_count = geometry.indices.size();
  resources.vertex_byte_size = vertex_byte_size;
  resources.index_byte_size = index_byte_size;
  resources.draws = std::move(geometry.draws);
  return {};
}

} // namespace

VkBufferCreateInfo vulkan_rounded_rect_buffer_create_info(
    std::size_t byte_size,
    VkBufferUsageFlags usage) {
  return VkBufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = byte_size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
}

Result<void> vulkan_upload_rounded_rect_buffers(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const RoundedRectDraw> rounded_rects,
    VulkanRoundedRectBufferResources& resources) {
  return upload_geometry(
      physical_device,
      device,
      vulkan_build_rounded_rect_geometry(rounded_rects),
      resources);
}

Result<void> vulkan_upload_solid_rect_buffers(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const SolidRect> rects,
    VulkanRoundedRectBufferResources& resources) {
  return upload_geometry(
      physical_device,
      device,
      vulkan_build_solid_rect_geometry(rects),
      resources);
}

void vulkan_destroy_rounded_rect_buffers(
    VkDevice device,
    VulkanRoundedRectBufferResources& resources) {
  if (device == VK_NULL_HANDLE) {
    return;
  }
  if (resources.index_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, resources.index_buffer, nullptr);
  }
  if (resources.index_memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, resources.index_memory, nullptr);
  }
  if (resources.vertex_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, resources.vertex_buffer, nullptr);
  }
  if (resources.vertex_memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, resources.vertex_memory, nullptr);
  }
  resources = {};
}

} // namespace cgpui
