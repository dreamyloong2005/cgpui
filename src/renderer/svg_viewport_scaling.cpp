#include "cgpui/renderer/svg_viewport_scaling.hpp"

#include <cmath>
#include <limits>

namespace cgpui {
namespace {

bool positive_finite(float value) {
  return std::isfinite(value) && value > 0.0F;
}

bool empty_size(Size size) {
  return size.width == 0.0F && size.height == 0.0F;
}

bool positive_finite_size(Size size) {
  return positive_finite(size.width) && positive_finite(size.height);
}

} // namespace

bool SvgViewportScalingPlan::ready() const {
  return status == SvgViewportScalingStatus::ready;
}

SvgViewportScalingPlan plan_svg_viewport_scaling(
    const SvgViewportScalingRequest& request) {
  SvgViewportScalingPlan plan;
  if (!positive_finite_size(request.logical_size)) {
    return plan;
  }

  const Size viewport_size =
      empty_size(request.viewport_size) ? request.logical_size
                                        : request.viewport_size;
  if (!positive_finite_size(viewport_size)) {
    plan.status = SvgViewportScalingStatus::invalid_viewport_size;
    return plan;
  }
  if (!positive_finite(request.scale.value)) {
    plan.status = SvgViewportScalingStatus::invalid_scale;
    return plan;
  }

  const double pixel_width = std::ceil(
      static_cast<double>(viewport_size.width) * request.scale.value);
  const double pixel_height = std::ceil(
      static_cast<double>(viewport_size.height) * request.scale.value);
  if (!std::isfinite(pixel_width) || !std::isfinite(pixel_height) ||
      pixel_width > svg_rasterization_max_dimension ||
      pixel_height > svg_rasterization_max_dimension) {
    plan.status = SvgViewportScalingStatus::exceeds_limits;
    return plan;
  }

  const auto width = static_cast<std::uint32_t>(pixel_width);
  const auto height = static_cast<std::uint32_t>(pixel_height);
  const std::size_t stride = static_cast<std::size_t>(width) * 4U;
  if (height > 0 && stride > svg_rasterization_max_bytes / height) {
    plan.status = SvgViewportScalingStatus::exceeds_limits;
    return plan;
  }
  const std::size_t byte_size = stride * height;
  if (byte_size > svg_rasterization_max_bytes ||
      stride > std::numeric_limits<std::uint32_t>::max()) {
    plan.status = SvgViewportScalingStatus::exceeds_limits;
    return plan;
  }

  plan.status = SvgViewportScalingStatus::ready;
  plan.logical_size = request.logical_size;
  plan.viewport_size = viewport_size;
  plan.scale = request.scale;
  plan.pixel_width = width;
  plan.pixel_height = height;
  plan.stride = static_cast<std::uint32_t>(stride);
  plan.byte_size = byte_size;
  plan.effective_scale_x = static_cast<float>(pixel_width) / viewport_size.width;
  plan.effective_scale_y =
      static_cast<float>(pixel_height) / viewport_size.height;
  return plan;
}

} // namespace cgpui
