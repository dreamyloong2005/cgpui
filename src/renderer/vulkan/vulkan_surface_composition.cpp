#include "vulkan_internal.hpp"
#include "vulkan_surface_composition_internal.hpp"

namespace cgpui {

RendererCompositeAlpha vulkan_renderer_composite_alpha(
    VkCompositeAlphaFlagBitsKHR alpha) {
  switch (alpha) {
    case VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR:
      return RendererCompositeAlpha::opaque;
    case VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR:
      return RendererCompositeAlpha::pre_multiplied;
    case VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR:
      return RendererCompositeAlpha::post_multiplied;
    case VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR:
      return RendererCompositeAlpha::inherit;
    default:
      return RendererCompositeAlpha::unknown;
  }
}

RendererSurfaceComposition vulkan_surface_composition(
    VkCompositeAlphaFlagsKHR supported,
    bool transparent) {
  return RendererSurfaceComposition{
      .supported = true,
      .transparent_requested = transparent,
      .supported_composite_alpha = static_cast<std::uint32_t>(supported),
      .composite_alpha = vulkan_renderer_composite_alpha(
          choose_vulkan_composite_alpha(supported, transparent)),
  };
}

RendererSurfaceComposition VulkanRendererState::surface_composition() const {
  return surface_composition_;
}

// Swapchain recreation reuses the resize path, which restores the previous
// descriptor if the new swapchain cannot be created.
Result<RendererSurfaceComposition> VulkanRendererState::set_transparent_surface(
    bool transparent) {
  if (auto live = require_live_device(); !live) return std::unexpected(live.error());
  if (descriptor_.transparent_background == transparent &&
      surface_composition_.supported) {
    return surface_composition_;
  }
  const bool previous = descriptor_.transparent_background;
  descriptor_.transparent_background = transparent;
  if (auto result = resize(descriptor_.framebuffer_size, descriptor_.scale);
      !result) {
    descriptor_.transparent_background = previous;
    return std::unexpected(result.error());
  }
  return surface_composition_;
}

} // namespace cgpui
