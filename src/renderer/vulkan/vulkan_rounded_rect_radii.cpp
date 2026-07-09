#include "vulkan_rounded_rect_radii_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

float fit_scale(float limit, float sum) {
  return sum > 0.0F ? limit / sum : 1.0F;
}

} // namespace

VulkanRoundedRectRadiiResolution vulkan_resolve_rounded_rect_radii(
    Size size,
    BorderRadii requested) {
  const BorderRadii clamped = BorderRadii::corners(
      std::max(0.0F, requested.top_left),
      std::max(0.0F, requested.top_right),
      std::max(0.0F, requested.bottom_right),
      std::max(0.0F, requested.bottom_left));
  if (size.width <= 0.0F || size.height <= 0.0F) {
    return VulkanRoundedRectRadiiResolution{
        .radii = {},
        .scale = 0.0F,
        .normalized = true,
    };
  }

  const float scale = std::clamp(
      std::min({
          1.0F,
          fit_scale(size.width, clamped.top_left + clamped.top_right),
          fit_scale(size.width, clamped.bottom_left + clamped.bottom_right),
          fit_scale(size.height, clamped.top_left + clamped.bottom_left),
          fit_scale(size.height, clamped.top_right + clamped.bottom_right),
      }),
      0.0F,
      1.0F);
  const bool clamped_negative =
      clamped.top_left != requested.top_left ||
      clamped.top_right != requested.top_right ||
      clamped.bottom_right != requested.bottom_right ||
      clamped.bottom_left != requested.bottom_left;
  return VulkanRoundedRectRadiiResolution{
      .radii = BorderRadii::corners(
          clamped.top_left * scale,
          clamped.top_right * scale,
          clamped.bottom_right * scale,
          clamped.bottom_left * scale),
      .scale = scale,
      .normalized = clamped_negative || scale < 1.0F,
  };
}

} // namespace cgpui
