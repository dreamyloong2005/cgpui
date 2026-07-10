#pragma once

#include "vulkan_rounded_rect_geometry_internal.hpp"

namespace cgpui {

[[nodiscard]] VulkanRoundedRectGeometry vulkan_build_solid_rect_geometry(
    std::span<const SolidRect> rects);

} // namespace cgpui
