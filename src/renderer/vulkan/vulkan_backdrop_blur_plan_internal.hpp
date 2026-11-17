#pragma once

#include "cgpui/renderer/renderer_commands.hpp"
#include "vulkan_platform_internal.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace cgpui {

// Composite quad vertex. `region` = {scene origin x, scene origin y,
// downsample factor, composed opacity}; rect/radii are in local space.
struct VulkanBackdropBlurVertex {
  std::array<float, 2> position{};
  std::array<float, 2> local{};
  std::array<float, 4> rect{};
  std::array<float, 4> radii{};
  std::array<float, 4> tint{};
  std::array<float, 4> region{};
};

// One backdrop: blur `region` (scene pixels, the rect's device bounds grown
// by 3 sigma and clamped to the frame) downsampled by `factor` into
// `texels`, with a `radius`-tap Gaussian of `sigma` texels per direction.
struct VulkanBackdropBlurPass {
  std::size_t source_index = 0;
  VkRect2D region{};
  std::uint32_t factor = 1;
  VkExtent2D texels{};
  float sigma = 0.0F;
  int radius = 0;
  std::uint32_t first_vertex = 0;
  std::optional<Rect> clip_rect;
};

struct VulkanBackdropBlurPlan {
  std::vector<VulkanBackdropBlurVertex> vertices;
  std::vector<VulkanBackdropBlurPass> passes;
};

inline constexpr std::uint32_t kVulkanBackdropBlurVerticesPerPass = 6;
inline constexpr int kVulkanBackdropBlurMaxRadius = 48;

// 1 up to sigma 4 device px, 2 up to 16, 4 up to 48, else 8.
[[nodiscard]] std::uint32_t vulkan_backdrop_blur_downsample_factor(float sigma);
// Draws are device pixels; invisible or empty backdrops get no pass.
[[nodiscard]] VulkanBackdropBlurPlan vulkan_plan_backdrop_blur(
    std::span<const BackdropBlurDraw> backdrops,
    VkExtent2D extent);
// The scene format must be sampled with linear filtering and rendered to.
[[nodiscard]] RendererEffectSupport vulkan_backdrop_blur_support(
    VkFormatFeatureFlags optimal_tiling_features);
// Offscreen render passes a frame records: one scene pass per segment
// (backdrops + 1) and downsample + two blur passes per backdrop.
[[nodiscard]] std::size_t vulkan_backdrop_blur_offscreen_pass_count(
    std::size_t backdrop_count);

} // namespace cgpui
