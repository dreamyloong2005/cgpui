#pragma once

#include "cgpui/renderer/renderer_types.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace cgpui {

inline constexpr std::uint32_t svg_rasterization_max_dimension = 16384;
inline constexpr std::size_t svg_rasterization_max_bytes =
    512U * 1024U * 1024U;

enum class SvgRasterizationStrategy {
  rgba8_bitmap,
};

enum class SvgRasterizationPlanStatus {
  ready,
  invalid_asset_id,
  empty_source,
  invalid_logical_size,
  invalid_scale,
  exceeds_limits,
};

struct SvgRasterizationRequest {
  ImageAssetId asset_id;
  Size logical_size;
  std::string_view svg_source;
  DpiScale scale;
};

struct SvgRasterizationPlan {
  SvgRasterizationStrategy strategy =
      SvgRasterizationStrategy::rgba8_bitmap;
  SvgRasterizationPlanStatus status =
      SvgRasterizationPlanStatus::invalid_asset_id;
  ImageAssetDescriptor descriptor;

  [[nodiscard]] bool ready() const;
};

[[nodiscard]] SvgRasterizationPlan plan_svg_rasterization(
    const SvgRasterizationRequest& request);

} // namespace cgpui
