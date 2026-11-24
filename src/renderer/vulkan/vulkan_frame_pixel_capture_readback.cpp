#include "vulkan_frame_pixel_capture_internal.hpp"

#include <cstring>
#include <limits>
#include <utility>

namespace cgpui {
namespace {

bool bgra_format(VkFormat format) {
  return format == VK_FORMAT_B8G8R8A8_SRGB ||
         format == VK_FORMAT_B8G8R8A8_UNORM;
}

RendererFramePixelEncoding pixel_encoding(VkFormat format) {
  return format == VK_FORMAT_B8G8R8A8_SRGB ||
                 format == VK_FORMAT_R8G8B8A8_SRGB
             ? RendererFramePixelEncoding::srgb
             : RendererFramePixelEncoding::linear;
}

} // namespace

Result<void> vulkan_complete_frame_pixel_capture(
    VkDevice device,
    VkFence fence,
    VulkanFramePixelCaptureResources& resources,
    VkResult* native_failure) {
  if (native_failure) *native_failure = VK_SUCCESS;
  if (device == VK_NULL_HANDLE || fence == VK_NULL_HANDLE ||
      !resources.allocated() || resources.extent.width == 0 ||
      resources.extent.height == 0) {
    return std::unexpected(vulkan_error(
        ErrorCode::invalid_argument,
        "Vulkan frame pixel capture completion is invalid"));
  }
  const VkResult waited = vkWaitForFences(
      device, 1, &fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
  if (native_failure) *native_failure = waited;
  if (auto result = require_vk_success(waited,
          "vkWaitForFences for frame pixel capture failed");
      !result) {
    return result;
  }
  const std::size_t byte_size =
      static_cast<std::size_t>(resources.extent.width) *
      resources.extent.height * 4;
  RendererFramePixels pixels{
      .width = resources.extent.width,
      .height = resources.extent.height,
      .encoding = pixel_encoding(resources.format),
      .rgba8 = std::vector<std::uint8_t>(byte_size),
  };
  void* mapped = nullptr;
  const VkResult mapped_result = vkMapMemory(device, resources.memory, 0, byte_size, 0, &mapped);
  if (native_failure) *native_failure = mapped_result;
  if (auto result = require_vk_success(mapped_result,
          "vkMapMemory for frame pixel capture failed");
      !result) {
    return result;
  }
  std::memcpy(pixels.rgba8.data(), mapped, byte_size);
  vkUnmapMemory(device, resources.memory);
  if (bgra_format(resources.format)) {
    for (std::size_t offset = 0; offset < byte_size; offset += 4) {
      std::swap(pixels.rgba8[offset], pixels.rgba8[offset + 2]);
    }
  }
  resources.pixels = std::move(pixels);
  return {};
}

} // namespace cgpui
