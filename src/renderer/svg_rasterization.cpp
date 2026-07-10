#include "cgpui/renderer/svg_rasterization.hpp"

namespace cgpui {
namespace {

SvgRasterizationPlanStatus raster_status_for_viewport(
    SvgViewportScalingStatus status) {
  switch (status) {
    case SvgViewportScalingStatus::invalid_logical_size:
      return SvgRasterizationPlanStatus::invalid_logical_size;
    case SvgViewportScalingStatus::invalid_viewport_size:
      return SvgRasterizationPlanStatus::invalid_viewport_size;
    case SvgViewportScalingStatus::invalid_scale:
      return SvgRasterizationPlanStatus::invalid_scale;
    case SvgViewportScalingStatus::exceeds_limits:
      return SvgRasterizationPlanStatus::exceeds_limits;
    case SvgViewportScalingStatus::ready:
      return SvgRasterizationPlanStatus::ready;
  }
  return SvgRasterizationPlanStatus::exceeds_limits;
}

} // namespace

bool SvgRasterizationPlan::ready() const {
  return status == SvgRasterizationPlanStatus::ready;
}

bool SvgRasterizationResult::ready() const {
  return status == SvgRasterizationStatus::ready;
}

SvgRasterizationPlan plan_svg_rasterization(
    const SvgRasterizationRequest& request) {
  SvgRasterizationPlan plan;
  if (request.asset_id.value == 0) {
    return plan;
  }
  if (request.svg_source.empty()) {
    plan.status = SvgRasterizationPlanStatus::empty_source;
    return plan;
  }
  plan.viewport = plan_svg_viewport_scaling(SvgViewportScalingRequest{
      .logical_size = request.logical_size,
      .viewport_size = request.viewport_size,
      .scale = request.scale,
  });
  if (!plan.viewport.ready()) {
    plan.status = raster_status_for_viewport(plan.viewport.status);
    return plan;
  }

  plan.status = SvgRasterizationPlanStatus::ready;
  plan.descriptor = ImageAssetDescriptor{
      .id = request.asset_id,
      .logical_size = plan.viewport.viewport_size,
      .pixel_width = plan.viewport.pixel_width,
      .pixel_height = plan.viewport.pixel_height,
      .stride = plan.viewport.stride,
      .format = ImageFormat::rgba8_unorm,
      .byte_size = plan.viewport.byte_size,
  };
  return plan;
}

} // namespace cgpui
