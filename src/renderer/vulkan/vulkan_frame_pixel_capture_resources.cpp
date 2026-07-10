#include "vulkan_frame_pixel_capture_internal.hpp"

#include "vulkan_device_internal.hpp"

#include <limits>

namespace cgpui {
namespace {

Result<std::size_t> capture_byte_size(VkExtent2D extent) {
  const std::size_t row_byte_count =
      static_cast<std::size_t>(extent.width) * 4;
  if (extent.width == 0 || extent.height == 0 ||
      row_byte_count >
          std::numeric_limits<std::size_t>::max() / extent.height) {
    return std::unexpected(vulkan_error(
        ErrorCode::invalid_argument,
        "Vulkan frame pixel capture extent is invalid"));
  }
  return row_byte_count * extent.height;
}

Result<void> allocate_capture_buffer(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::size_t byte_size,
    VulkanFramePixelCaptureResources& resources) {
  const VkBufferCreateInfo buffer_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = byte_size,
      .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  if (auto result = require_vk_success(
          vkCreateBuffer(device, &buffer_info, nullptr, &resources.buffer),
          "vkCreateBuffer for frame pixel capture failed");
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
          "vkAllocateMemory for frame pixel capture failed");
      !result) {
    return result;
  }
  if (auto result = require_vk_success(
          vkBindBufferMemory(device, resources.buffer, resources.memory, 0),
          "vkBindBufferMemory for frame pixel capture failed");
      !result) {
    return result;
  }
  resources.byte_capacity = byte_size;
  return {};
}

} // namespace

bool VulkanFramePixelCaptureResources::allocated() const {
  return buffer != VK_NULL_HANDLE && memory != VK_NULL_HANDLE &&
         byte_capacity != 0;
}

bool VulkanFramePixelCaptureCommand::enabled() const {
  return image != VK_NULL_HANDLE && buffer != VK_NULL_HANDLE &&
         extent.width != 0 && extent.height != 0;
}

bool vulkan_frame_pixel_capture_format_supported(VkFormat format) {
  return format == VK_FORMAT_B8G8R8A8_SRGB ||
         format == VK_FORMAT_B8G8R8A8_UNORM ||
         format == VK_FORMAT_R8G8B8A8_SRGB ||
         format == VK_FORMAT_R8G8B8A8_UNORM;
}

Result<VulkanFramePixelCaptureCommand> vulkan_prepare_frame_pixel_capture(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkImage image,
    VkExtent2D extent,
    VkFormat format,
    VulkanFramePixelCaptureResources& resources) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE ||
      image == VK_NULL_HANDLE ||
      !vulkan_frame_pixel_capture_format_supported(format)) {
    return std::unexpected(vulkan_error(
        ErrorCode::unsupported_platform,
        "Vulkan frame pixel capture is unsupported for this swapchain"));
  }
  auto required_size = capture_byte_size(extent);
  if (!required_size) {
    return std::unexpected(required_size.error());
  }
  if (!resources.allocated() || resources.byte_capacity < *required_size) {
    vulkan_destroy_frame_pixel_capture(device, resources);
    if (auto result = allocate_capture_buffer(
            physical_device, device, *required_size, resources);
        !result) {
      vulkan_destroy_frame_pixel_capture(device, resources);
      return std::unexpected(result.error());
    }
  }
  resources.extent = extent;
  resources.format = format;
  return VulkanFramePixelCaptureCommand{
      .image = image,
      .buffer = resources.buffer,
      .extent = extent,
  };
}

void vulkan_destroy_frame_pixel_capture(
    VkDevice device,
    VulkanFramePixelCaptureResources& resources) {
  if (device != VK_NULL_HANDLE) {
    if (resources.buffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(device, resources.buffer, nullptr);
    }
    if (resources.memory != VK_NULL_HANDLE) {
      vkFreeMemory(device, resources.memory, nullptr);
    }
  }
  resources = {};
}

} // namespace cgpui
