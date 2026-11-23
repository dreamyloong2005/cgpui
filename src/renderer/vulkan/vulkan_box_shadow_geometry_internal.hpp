#pragma once

#include "cgpui/renderer/planning/box_shadow_geometry.hpp"
#include "cgpui/renderer/planning/box_shadow_parameters.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace cgpui {

using VulkanBoxShadowVertex = BoxShadowVertex;
using VulkanBoxShadowGeometry = BoxShadowGeometry;

// CSS box-shadow blur radius to Gaussian standard deviation (blur / 2).
[[nodiscard]] float vulkan_box_shadow_sigma(float blur_radius);
// CSS spread applied to one corner radius (css-backgrounds-3 7.1.1).
[[nodiscard]] float vulkan_box_shadow_spread_radius(float radius, float spread);
// The quad covers the shadow shape grown by 3 sigma.
[[nodiscard]] VulkanBoxShadowGeometry vulkan_build_box_shadow_geometry(
    std::span<const BoxShadowDraw> shadows);

} // namespace cgpui
