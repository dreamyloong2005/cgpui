#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "vulkan_rounded_rect_antialiasing_internal.hpp"
#include "vulkan_rounded_rect_radii_internal.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace cgpui {

struct VulkanRoundedRectVertex {
  std::array<float, 2> position{};
  std::array<float, 4> color{};
  float coverage = 1.0F;
};

struct VulkanRoundedRectDrawRange {
  std::size_t source_index = 0;
  std::size_t first_vertex = 0;
  std::size_t vertex_count = 0;
  std::size_t first_index = 0;
  std::size_t index_count = 0;
};

struct VulkanRoundedRectGeometry {
  std::vector<VulkanRoundedRectVertex> vertices;
  std::vector<std::uint32_t> indices;
  std::vector<VulkanRoundedRectDrawRange> draws;
};

[[nodiscard]] VulkanRoundedRectGeometry vulkan_build_rounded_rect_geometry(
    std::span<const RoundedRectDraw> rounded_rects,
    std::size_t corner_segment_count = 4,
    VulkanRoundedRectAntialiasingPolicy antialiasing =
        vulkan_default_rounded_rect_antialiasing_policy());

} // namespace cgpui
