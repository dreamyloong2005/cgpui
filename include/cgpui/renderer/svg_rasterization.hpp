#pragma once

#include "cgpui/renderer/svg_viewport_scaling.hpp"

#include <string_view>

namespace cgpui {

enum class SvgRasterizationStrategy {
  rgba8_bitmap,
};

enum class SvgRasterizationPlanStatus {
  ready,
  invalid_asset_id,
  empty_source,
  invalid_logical_size,
  invalid_viewport_size,
  invalid_scale,
  exceeds_limits,
};

struct SvgRasterizationRequest {
  ImageAssetId asset_id;
  Size logical_size;
  std::string_view svg_source;
  DpiScale scale;
  Size viewport_size;
};

struct SvgRasterizationPlan {
  SvgRasterizationStrategy strategy =
      SvgRasterizationStrategy::rgba8_bitmap;
  SvgRasterizationPlanStatus status =
      SvgRasterizationPlanStatus::invalid_asset_id;
  SvgViewportScalingPlan viewport;
  ImageAssetDescriptor descriptor;

  [[nodiscard]] bool ready() const;
};

enum class SvgRasterizationStatus {
  ready,
  invalid_request,
  invalid_svg,
  rasterization_failed,
};

struct SvgRasterizationResult {
  SvgRasterizationStatus status = SvgRasterizationStatus::invalid_request;
  SvgRasterizationPlan plan;
  ImageAsset image;

  [[nodiscard]] bool ready() const;
};

[[nodiscard]] SvgRasterizationPlan plan_svg_rasterization(
    const SvgRasterizationRequest& request);
[[nodiscard]] SvgRasterizationResult rasterize_svg(
    const SvgRasterizationRequest& request);

} // namespace cgpui
