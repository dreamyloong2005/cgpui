#pragma once

#include "cgpui/renderer/renderer_types.hpp"

#include <cstddef>
#include <cstdint>

namespace cgpui {

inline constexpr std::uint32_t svg_rasterization_max_dimension = 16384;
inline constexpr std::size_t svg_rasterization_max_bytes =
    512U * 1024U * 1024U;

enum class SvgViewportScalingStatus {
  ready,
  invalid_logical_size,
  invalid_viewport_size,
  invalid_scale,
  exceeds_limits,
};

struct SvgViewportScalingRequest {
  Size logical_size;
  Size viewport_size;
  DpiScale scale;
};

struct SvgViewportScalingPlan {
  SvgViewportScalingStatus status =
      SvgViewportScalingStatus::invalid_logical_size;
  Size logical_size;
  Size viewport_size;
  DpiScale scale;
  std::uint32_t pixel_width = 0;
  std::uint32_t pixel_height = 0;
  std::uint32_t stride = 0;
  std::size_t byte_size = 0;
  float effective_scale_x = 0.0F;
  float effective_scale_y = 0.0F;

  [[nodiscard]] bool ready() const;
};

[[nodiscard]] SvgViewportScalingPlan plan_svg_viewport_scaling(
    const SvgViewportScalingRequest& request);

} // namespace cgpui
