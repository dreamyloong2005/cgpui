#pragma once

#include "../planning/rounded_rect_radii_internal.hpp"

#include "cgpui/core/paint_values.hpp"

namespace cgpui {

using VulkanRoundedRectRadiiResolution = RoundedRectRadiiResolution;

[[nodiscard]] VulkanRoundedRectRadiiResolution
vulkan_resolve_rounded_rect_radii(Size size, BorderRadii requested);

} // namespace cgpui
