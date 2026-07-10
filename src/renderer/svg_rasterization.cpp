#include "cgpui/renderer/svg_rasterization.hpp"

#include <cmath>
#include <limits>

namespace cgpui {
namespace {

bool positive_finite(float value) {
  return std::isfinite(value) && value > 0.0F;
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
  if (!positive_finite(request.logical_size.width) ||
      !positive_finite(request.logical_size.height)) {
    plan.status = SvgRasterizationPlanStatus::invalid_logical_size;
    return plan;
  }
  if (!positive_finite(request.scale.value)) {
    plan.status = SvgRasterizationPlanStatus::invalid_scale;
    return plan;
  }

  const double pixel_width = std::ceil(
      static_cast<double>(request.logical_size.width) * request.scale.value);
  const double pixel_height = std::ceil(
      static_cast<double>(request.logical_size.height) * request.scale.value);
  if (!std::isfinite(pixel_width) || !std::isfinite(pixel_height) ||
      pixel_width > svg_rasterization_max_dimension ||
      pixel_height > svg_rasterization_max_dimension) {
    plan.status = SvgRasterizationPlanStatus::exceeds_limits;
    return plan;
  }

  const auto width = static_cast<std::uint32_t>(pixel_width);
  const auto height = static_cast<std::uint32_t>(pixel_height);
  const std::size_t stride = static_cast<std::size_t>(width) * 4U;
  if (height > 0 && stride > svg_rasterization_max_bytes / height) {
    plan.status = SvgRasterizationPlanStatus::exceeds_limits;
    return plan;
  }
  const std::size_t byte_size = stride * height;
  if (byte_size > svg_rasterization_max_bytes ||
      stride > std::numeric_limits<std::uint32_t>::max()) {
    plan.status = SvgRasterizationPlanStatus::exceeds_limits;
    return plan;
  }

  plan.status = SvgRasterizationPlanStatus::ready;
  plan.descriptor = ImageAssetDescriptor{
      .id = request.asset_id,
      .logical_size = request.logical_size,
      .pixel_width = width,
      .pixel_height = height,
      .stride = static_cast<std::uint32_t>(stride),
      .format = ImageFormat::rgba8_unorm,
      .byte_size = byte_size,
  };
  return plan;
}

} // namespace cgpui
