#pragma once

#include "vulkan_backdrop_blur_resources_internal.hpp"
#include "vulkan_frame_draw_order_internal.hpp"

#include <functional>
#include <span>

namespace cgpui {

struct VulkanBackdropBlurFrame {
  const VulkanBackdropBlurResources* resources = nullptr;
  std::span<const VulkanBackdropBlurPass> passes;

  [[nodiscard]] bool active() const {
    return resources != nullptr && resources->ready() && !passes.empty();
  }
};

struct VulkanBackdropBlurPresentTarget {
  VkRenderPass render_pass = VK_NULL_HANDLE;
  VkFramebuffer framebuffer = VK_NULL_HANDLE;
  VkExtent2D extent{};
  Color clear_color;
};

// Records the ordinary draws between backdrops.
using VulkanFrameDrawSegmentRecorder =
    std::function<void(std::span<const VulkanFrameDrawOrderEntry>)>;

// Draws the frame into the offscreen scene in segments split at backdrop
// entries: scene pass, then downsample + horizontal + vertical blur of the
// backdrop region, then the next scene pass starts with the composite.
// The swapchain pass copies the scene. Returns the offscreen pass count.
std::size_t vulkan_record_backdrop_blur_frame(
    VkCommandBuffer command_buffer,
    const VulkanBackdropBlurFrame& frame,
    const VulkanBackdropBlurPresentTarget& present,
    std::span<const VulkanFrameDrawOrderEntry> draw_order,
    const VulkanFrameDrawSegmentRecorder& record_segment);

} // namespace cgpui
