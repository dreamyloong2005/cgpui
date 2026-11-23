#pragma once

#include "../planning/rounded_rect_stroke_internal.hpp"

#include "cgpui/renderer/renderer_commands.hpp"

namespace cgpui {

using VulkanRoundedRectStrokeResolution = RoundedRectStrokeResolution;

[[nodiscard]] VulkanRoundedRectStrokeResolution
vulkan_resolve_rounded_rect_stroke(
    const RoundedRectDraw& draw,
    BorderRadii normalized_outer_radii);

} // namespace cgpui
