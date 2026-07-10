#include "vulkan_frame_geometry_buffer_internal.hpp"

#include "vulkan_device_internal.hpp"

#include <algorithm>
#include <cstring>
#include <limits>

namespace cgpui {
namespace {

bool frame_geometry_usage_valid(VkBufferUsageFlags usage) {
  return usage == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ||
         usage == VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
}

std::size_t grown_capacity(
    std::size_t current_capacity,
    std::size_t required_byte_size) {
  if (current_capacity >
      std::numeric_limits<std::size_t>::max() / 2) {
    return required_byte_size;
  }
  return std::max(required_byte_size, current_capacity * 2);
}

Result<void> map_frame_geometry_bytes(
    VkDevice device,
    const void* data,
    std::size_t byte_size,
    VkDeviceMemory memory) {
  void* mapped = nullptr;
  if (auto result = require_vk_success(
          vkMapMemory(device, memory, 0, byte_size, 0, &mapped),
          "vkMapMemory for frame geometry failed");
      !result) {
    return result;
  }
  std::memcpy(mapped, data, byte_size);
  vkUnmapMemory(device, memory);
  return {};
}

Result<void> allocate_frame_geometry_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const VulkanFrameGeometryBufferPlan& plan,
    VulkanFrameGeometryBufferResources& resources) {
  const VkBufferCreateInfo buffer_info =
      vulkan_frame_geometry_buffer_create_info(
          plan.allocation_byte_size, plan.usage);
  if (auto result = require_vk_success(
          vkCreateBuffer(device, &buffer_info, nullptr, &resources.buffer),
          "vkCreateBuffer for frame geometry failed");
      !result) {
    return result;
  }
  VkMemoryRequirements requirements{};
  vkGetBufferMemoryRequirements(device, resources.buffer, &requirements);
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
          vkAllocateMemory(
              device, &allocation_info, nullptr, &resources.memory),
          "vkAllocateMemory for frame geometry failed");
      !result) {
    return result;
  }
  if (auto result = require_vk_success(
          vkBindBufferMemory(device, resources.buffer, resources.memory, 0),
          "vkBindBufferMemory for frame geometry failed");
      !result) {
    return result;
  }
  resources.byte_capacity = plan.allocation_byte_size;
  resources.usage = plan.usage;
  return {};
}

} // namespace

VulkanFrameGeometryBufferPlan vulkan_plan_frame_geometry_buffer(
    const VulkanFrameGeometryBufferResources& resources,
    std::size_t required_byte_size,
    VkBufferUsageFlags usage) {
  VulkanFrameGeometryBufferPlan plan{
      .required_byte_size = required_byte_size,
      .allocation_byte_size = resources.byte_capacity,
      .usage = usage,
  };
  if (!frame_geometry_usage_valid(usage)) {
    return plan;
  }
  if (required_byte_size == 0) {
    plan.action = VulkanFrameGeometryBufferAction::retain_empty;
    return plan;
  }
  if (resources.allocated() && resources.usage == usage) {
    if (resources.byte_capacity >= required_byte_size) {
      plan.action = VulkanFrameGeometryBufferAction::reuse;
      return plan;
    }
    plan.action = VulkanFrameGeometryBufferAction::replace;
    plan.allocation_byte_size =
        grown_capacity(resources.byte_capacity, required_byte_size);
    return plan;
  }
  plan.action = VulkanFrameGeometryBufferAction::replace;
  plan.allocation_byte_size = required_byte_size;
  return plan;
}

VkBufferCreateInfo vulkan_frame_geometry_buffer_create_info(
    std::size_t byte_size,
    VkBufferUsageFlags usage) {
  return VkBufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = byte_size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
}

Result<void> vulkan_upload_frame_geometry_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const void* data,
    std::size_t byte_size,
    VkBufferUsageFlags usage,
    VulkanFrameGeometryBufferResources& resources) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "frame geometry upload requires Vulkan devices"));
  }
  const VulkanFrameGeometryBufferPlan plan =
      vulkan_plan_frame_geometry_buffer(resources, byte_size, usage);
  if (!plan.valid() || (byte_size != 0 && data == nullptr)) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "frame geometry upload request is invalid"));
  }
  if (plan.action == VulkanFrameGeometryBufferAction::retain_empty) {
    return {};
  }
  if (plan.action == VulkanFrameGeometryBufferAction::reuse) {
    return map_frame_geometry_bytes(device, data, byte_size, resources.memory);
  }

  VulkanFrameGeometryBufferResources replacement;
  if (auto result = allocate_frame_geometry_buffer(
          physical_device, device, plan, replacement);
      !result) {
    vulkan_destroy_frame_geometry_buffer(device, replacement);
    return result;
  }
  if (auto result =
          map_frame_geometry_bytes(device, data, byte_size, replacement.memory);
      !result) {
    vulkan_destroy_frame_geometry_buffer(device, replacement);
    return result;
  }
  vulkan_destroy_frame_geometry_buffer(device, resources);
  resources = replacement;
  return {};
}

void vulkan_destroy_frame_geometry_buffer(
    VkDevice device,
    VulkanFrameGeometryBufferResources& resources) {
  if (device == VK_NULL_HANDLE) {
    return;
  }
  if (resources.buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, resources.buffer, nullptr);
  }
  if (resources.memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, resources.memory, nullptr);
  }
  resources = {};
}

} // namespace cgpui
