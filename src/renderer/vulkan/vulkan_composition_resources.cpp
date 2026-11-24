#include "vulkan_internal.hpp"
#include "vulkan_surface_composition_internal.hpp"

namespace cgpui {

VkImageLayout VulkanRendererState::presented_image_layout() const {
  return composition_surface() ? vulkan_composition_frame_layout
                               : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

// The swapchain resources of a composition surface: one image, the
// presenter's imported texture, with the same targets as a swapchain.
// Resizing replaces that image, so the resources installed over it go
// first; when this fails, presentation stays blocked until a resize.
Result<VulkanSwapchainResources> VulkanRendererState::create_composition_resources(
    bool* retired_old_swapchain) {
  destroy_swapchain();
  if (retired_old_swapchain != nullptr) *retired_old_swapchain = true;
  const VkExtent2D extent{
      .width = static_cast<std::uint32_t>(
          std::max(1L, std::lround(descriptor_.framebuffer_size.width))),
      .height = static_cast<std::uint32_t>(
          std::max(1L, std::lround(descriptor_.framebuffer_size.height))),
  };
  if (composition_ == nullptr) {
    auto surface = require_win32_surface(descriptor_.native_surface);
    if (!surface) return std::unexpected(surface.error());
    auto presenter = VulkanCompositionPresenter::create(
        {.physical_device = physical_device_,
         .device = device_,
         .command_pool = command_pool_,
         .queue_family = graphics_queue_family_},
        *surface, extent);
    if (!presenter) return std::unexpected(presenter.error());
    composition_ = std::move(*presenter);
  } else if (auto resized = composition_->resize(extent); !resized) {
    return std::unexpected(resized.error());
  }

  VulkanSwapchainResources resources{
      .format = vulkan_composition_format,
      .extent = composition_->extent(),
      .image_usage = vulkan_composition_usage,
      .images = {composition_->image()},
  };
  try {
    if (auto result = create_swapchain_targets(resources); !result) {
      destroy_swapchain_resources(resources);
      return std::unexpected(result.error());
    }
    // DXGI_ALPHA_MODE_PREMULTIPLIED, whatever the request.
    surface_composition_ = vulkan_surface_composition(
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        descriptor_.transparent_background);
    return resources;
  } catch (...) {
    destroy_swapchain_resources(resources);
    throw;
  }
}

} // namespace cgpui
