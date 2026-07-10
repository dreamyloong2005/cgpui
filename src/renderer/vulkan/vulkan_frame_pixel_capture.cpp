#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::request_pixel_capture() const {
  if ((swapchain_image_usage_ & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0 ||
      !vulkan_frame_pixel_capture_format_supported(swapchain_format_)) {
    return std::unexpected(vulkan_error(
        ErrorCode::unsupported_platform,
        "Vulkan swapchain does not support frame pixel capture"));
  }
  return {};
}

const RendererFramePixels* VulkanRendererState::last_frame_pixels() const {
  return frame_pixel_capture_.pixels.valid()
             ? &frame_pixel_capture_.pixels
             : nullptr;
}

Result<VulkanFramePixelCaptureCommand>
VulkanRendererState::prepare_frame_pixel_capture(
    bool capture_requested,
    VkImage image) {
  if (!capture_requested) {
    return VulkanFramePixelCaptureCommand{};
  }
  if (auto result = request_pixel_capture(); !result) {
    return std::unexpected(result.error());
  }
  return vulkan_prepare_frame_pixel_capture(
      physical_device_,
      device_,
      image,
      swapchain_extent_,
      swapchain_format_,
      frame_pixel_capture_);
}

Result<void> VulkanRendererState::complete_frame_pixel_capture(
    const VulkanFramePixelCaptureCommand& capture) {
  return capture.enabled()
             ? vulkan_complete_frame_pixel_capture(
                   device_, in_flight_, frame_pixel_capture_)
             : Result<void>{};
}

} // namespace cgpui
