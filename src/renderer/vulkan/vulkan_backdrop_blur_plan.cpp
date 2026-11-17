#include "vulkan_backdrop_blur_plan_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"
#include "vulkan_composition_opacity_internal.hpp"
#include "vulkan_composition_transform_internal.hpp"
#include "vulkan_rounded_rect_radii_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {
namespace {

// Device bounds of the transformed rect grown by `margin`, clamped.
std::optional<VkRect2D> blur_region(
    std::span<const Point, 4> corners, float margin, VkExtent2D extent) {
  float min_x = corners[0].x;
  float min_y = corners[0].y;
  float max_x = corners[0].x;
  float max_y = corners[0].y;
  for (const Point corner : corners) {
    min_x = (std::min)(min_x, corner.x);
    min_y = (std::min)(min_y, corner.y);
    max_x = (std::max)(max_x, corner.x);
    max_y = (std::max)(max_y, corner.y);
  }
  const float left = (std::max)(0.0F, std::floor(min_x - margin));
  const float top = (std::max)(0.0F, std::floor(min_y - margin));
  const float right =
      (std::min)(static_cast<float>(extent.width), std::ceil(max_x + margin));
  const float bottom =
      (std::min)(static_cast<float>(extent.height), std::ceil(max_y + margin));
  if (!(right > left) || !(bottom > top)) {
    return std::nullopt;
  }
  return VkRect2D{
      .offset = {.x = static_cast<std::int32_t>(left),
                 .y = static_cast<std::int32_t>(top)},
      .extent = {.width = static_cast<std::uint32_t>(right - left),
                 .height = static_cast<std::uint32_t>(bottom - top)},
  };
}

std::uint32_t ceil_div(std::uint32_t value, std::uint32_t divisor) {
  return (value + divisor - 1) / divisor;
}

} // namespace

std::uint32_t vulkan_backdrop_blur_downsample_factor(float sigma) {
  if (!(sigma > 4.0F)) return 1;
  if (sigma <= 16.0F) return 2;
  if (sigma <= 48.0F) return 4;
  return 8;
}

VulkanBackdropBlurPlan vulkan_plan_backdrop_blur(
    std::span<const BackdropBlurDraw> backdrops,
    VkExtent2D extent) {
  VulkanBackdropBlurPlan plan;
  for (std::size_t index = 0; index < backdrops.size(); ++index) {
    const BackdropBlurDraw& draw = backdrops[index];
    const float opacity =
        vulkan_resolve_composed_opacity(draw.metadata, draw.composition_stack);
    if (draw.rect.size.width <= 0.0F || draw.rect.size.height <= 0.0F ||
        opacity <= 0.0F) {
      continue;
    }
    const float sigma = std::isfinite(draw.blur_radius)
                            ? (std::max)(0.0F, draw.blur_radius)
                            : 0.0F;
    const Rect rect = draw.rect;
    // The quad is 1 px larger so the mask's antialiased edge is not cut.
    const float x0 = rect.origin.x - 1.0F;
    const float y0 = rect.origin.y - 1.0F;
    const float x1 = rect.origin.x + rect.size.width + 1.0F;
    const float y1 = rect.origin.y + rect.size.height + 1.0F;
    const std::array<Point, 4> local{
        Point{.x = x0, .y = y0},
        Point{.x = x1, .y = y0},
        Point{.x = x1, .y = y1},
        Point{.x = x0, .y = y1},
    };
    std::array<Point, 4> device{};
    for (std::size_t corner = 0; corner < local.size(); ++corner) {
      device[corner] = vulkan_apply_composed_transform(
          local[corner], draw.metadata, draw.composition_stack);
    }
    const std::optional<VkRect2D> region =
        blur_region(device, std::ceil(sigma * 3.0F), extent);
    if (!region.has_value()) {
      continue;
    }
    const std::uint32_t factor = vulkan_backdrop_blur_downsample_factor(sigma);
    // The box downsample already has variance (f^2 - 1) / 12 px^2.
    const float box_variance =
        static_cast<float>(factor * factor - 1) / 12.0F;
    const float texel_sigma =
        std::sqrt((std::max)(0.0F, sigma * sigma - box_variance)) /
        static_cast<float>(factor);
    const BorderRadii radii =
        vulkan_resolve_rounded_rect_radii(rect.size, draw.radius).radii;
    // No tint mixes nothing (Color{} would be opaque black).
    const Color tint = draw.tint.value_or(Color{.a = 0.0F});
    VulkanBackdropBlurPass pass{
        .source_index = index,
        .region = *region,
        .factor = factor,
        .texels = {.width = ceil_div(region->extent.width, factor),
                   .height = ceil_div(region->extent.height, factor)},
        .sigma = texel_sigma,
        .radius = (std::min)(kVulkanBackdropBlurMaxRadius,
                             static_cast<int>(std::ceil(texel_sigma * 3.0F))),
        .first_vertex = static_cast<std::uint32_t>(plan.vertices.size()),
        .clip_rect =
            vulkan_resolve_effective_clip_rect(draw.clip_rect, draw.clip_stack),
    };
    for (const std::size_t corner : {0U, 1U, 2U, 2U, 3U, 0U}) {
      plan.vertices.push_back(VulkanBackdropBlurVertex{
          .position = {device[corner].x, device[corner].y},
          .local = {local[corner].x, local[corner].y},
          .rect = {rect.origin.x, rect.origin.y,
                   rect.origin.x + rect.size.width,
                   rect.origin.y + rect.size.height},
          .radii = {radii.top_left, radii.top_right, radii.bottom_right,
                    radii.bottom_left},
          .tint = {tint.r, tint.g, tint.b, tint.a},
          .region = {static_cast<float>(region->offset.x),
                     static_cast<float>(region->offset.y),
                     static_cast<float>(factor), opacity},
      });
    }
    plan.passes.push_back(pass);
  }
  return plan;
}

RendererEffectSupport vulkan_backdrop_blur_support(
    VkFormatFeatureFlags optimal_tiling_features) {
  constexpr VkFormatFeatureFlags required =
      VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
      VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
      VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
  if ((optimal_tiling_features & required) != required) {
    return RendererEffectSupport{
        .supported = false,
        .reason = "swapchain format cannot be rendered to and sampled with "
                  "linear filtering; backdrop blur draws the tint only",
    };
  }
  return RendererEffectSupport{.supported = true};
}

std::size_t vulkan_backdrop_blur_offscreen_pass_count(
    std::size_t backdrop_count) {
  return backdrop_count == 0 ? 0 : backdrop_count * 4 + 1;
}

} // namespace cgpui
