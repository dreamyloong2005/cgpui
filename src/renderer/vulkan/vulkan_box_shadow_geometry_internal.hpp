#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace cgpui {

// One shadow is one quad. `position` is the transformed device position;
// the fragment shader evaluates the shadow at the untransformed `local`
// point against the shadow and box shapes, so an affine transform scales
// and rotates the shadow with its box. Rects are {min x, min y, max x,
// max y}; radii are {top left, top right, bottom right, bottom left}.
struct VulkanBoxShadowVertex {
  std::array<float, 2> position{};
  std::array<float, 2> local{};
  std::array<float, 4> color{};
  std::array<float, 4> shadow_rect{};
  std::array<float, 4> shadow_radii{};
  std::array<float, 4> box_rect{};
  std::array<float, 4> box_radii{};
  float sigma = 0.0F;
};

struct VulkanBoxShadowGeometry {
  std::vector<VulkanBoxShadowVertex> vertices;
  std::vector<std::uint32_t> indices;
  std::vector<VulkanRoundedRectDrawRange> draws;
};

// CSS box-shadow blur radius to Gaussian standard deviation (blur / 2).
[[nodiscard]] float vulkan_box_shadow_sigma(float blur_radius);
// CSS spread applied to one corner radius (css-backgrounds-3 7.1.1).
[[nodiscard]] float vulkan_box_shadow_spread_radius(float radius, float spread);
// The quad covers the shadow shape grown by 3 sigma.
[[nodiscard]] VulkanBoxShadowGeometry vulkan_build_box_shadow_geometry(
    std::span<const BoxShadowDraw> shadows);

} // namespace cgpui
