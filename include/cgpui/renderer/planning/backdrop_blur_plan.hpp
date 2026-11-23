#pragma once

#include "cgpui/renderer/renderer_effect_draws.hpp"
#include "cgpui/renderer/planning/backdrop_blur_vertex.hpp"
#include "cgpui/renderer/planning/backdrop_blur_pass.hpp"

#include <span>
#include <vector>

namespace cgpui {

struct BackdropBlurPlan {
  std::vector<BackdropBlurVertex> vertices;
  std::vector<BackdropBlurPass> passes;
};

// Draws are device pixels; invisible or empty backdrops get no pass.
// Extents above INT32_MAX in either dimension cannot be represented by
// signed scissor coordinates and return an empty plan. Native backends
// enforce their own, typically smaller framebuffer limits separately.
[[nodiscard]] BackdropBlurPlan plan_backdrop_blur(
    std::span<const BackdropBlurDraw> backdrops, PixelExtent extent);

} // namespace cgpui
