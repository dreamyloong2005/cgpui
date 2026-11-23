#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/renderer/planning/pixel_scissor.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace cgpui {

// Blur the scene-pixel region downsampled by factor into texels, with a
// radius-tap Gaussian of sigma texels in each direction.
struct BackdropBlurPass {
  std::size_t source_index = 0;
  PixelScissor region;
  std::uint32_t factor = 1;
  PixelExtent texels;
  float sigma = 0.0F;
  int radius = 0;
  std::uint32_t first_vertex = 0;
  std::optional<Rect> clip_rect;
};

} // namespace cgpui
