#pragma once

#include "cgpui/renderer/renderer_frame_pixels.hpp"
#include "vulkan_platform_internal.hpp"

namespace cgpui {

struct VulkanFramePixelCaptureResources {
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  std::size_t byte_capacity = 0;
  VkExtent2D extent{};
  VkFormat format = VK_FORMAT_UNDEFINED;
  RendererFramePixels pixels;

  [[nodiscard]] bool allocated() const;
};

struct VulkanFramePixelCaptureCommand {
  VkImage image = VK_NULL_HANDLE;
  VkBuffer buffer = VK_NULL_HANDLE;
  VkExtent2D extent{};

  [[nodiscard]] bool enabled() const;
};

[[nodiscard]] bool vulkan_frame_pixel_capture_format_supported(VkFormat format);
Result<VulkanFramePixelCaptureCommand> vulkan_prepare_frame_pixel_capture(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkImage image,
    VkExtent2D extent,
    VkFormat format,
    VulkanFramePixelCaptureResources& resources);
void vulkan_record_frame_pixel_capture(
    VkCommandBuffer command_buffer,
    const VulkanFramePixelCaptureCommand& capture);
Result<void> vulkan_complete_frame_pixel_capture(
    VkDevice device,
    VkFence fence,
    VulkanFramePixelCaptureResources& resources);
void vulkan_destroy_frame_pixel_capture(
    VkDevice device,
    VulkanFramePixelCaptureResources& resources);

} // namespace cgpui
