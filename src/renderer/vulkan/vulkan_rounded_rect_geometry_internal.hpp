#pragma once

#include "cgpui/renderer/planning/rounded_rect_geometry.hpp"

#include "cgpui/renderer/renderer_commands.hpp"
#include "vulkan_rounded_rect_antialiasing_internal.hpp"
#include "vulkan_rounded_rect_vertex_internal.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace cgpui {

using VulkanRoundedRectDrawRange = RectDrawRange;

using VulkanRoundedRectGeometry = RectGeometry;

[[nodiscard]] VulkanRoundedRectGeometry vulkan_build_rounded_rect_geometry(
    std::span<const RoundedRectDraw> rounded_rects,
    std::size_t corner_segment_count = 4,
    VulkanRoundedRectAntialiasingPolicy antialiasing =
        vulkan_default_rounded_rect_antialiasing_policy());

} // namespace cgpui
