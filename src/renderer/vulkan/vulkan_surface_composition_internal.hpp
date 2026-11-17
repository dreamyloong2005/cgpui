#pragma once

#include "vulkan_platform_internal.hpp"

#include "cgpui/renderer/renderer_surface_composition.hpp"

namespace cgpui {

[[nodiscard]] RendererCompositeAlpha vulkan_renderer_composite_alpha(
    VkCompositeAlphaFlagBitsKHR alpha);

// Builds the composition report for a swapchain created from `supported`
// with `transparent` requested.
[[nodiscard]] RendererSurfaceComposition vulkan_surface_composition(
    VkCompositeAlphaFlagsKHR supported,
    bool transparent);

} // namespace cgpui
