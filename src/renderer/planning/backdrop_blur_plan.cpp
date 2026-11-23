#include "cgpui/renderer/planning/backdrop_blur_plan.hpp"
#include "cgpui/renderer/planning/backdrop_blur_parameters.hpp"

#include "cgpui/renderer/planning/clip_scissor.hpp"
#include "cgpui/renderer/planning/composition_opacity.hpp"
#include "cgpui/renderer/planning/composition_transform.hpp"
#include "rounded_rect_radii_internal.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace cgpui {
namespace {

// Device bounds of the transformed rect grown by `margin`, clamped.
template <typename Scalar>
std::optional<PixelScissor> blur_region(
    std::span<const Point, 4> corners, float margin, PixelExtent extent) {
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
  const Scalar left = (std::max)(Scalar{0}, std::floor(static_cast<Scalar>(min_x) - margin));
  const Scalar top = (std::max)(Scalar{0}, std::floor(static_cast<Scalar>(min_y) - margin));
  const Scalar right =
      (std::min)(static_cast<Scalar>(extent.width), std::ceil(static_cast<Scalar>(max_x) + margin));
  const Scalar bottom =
      (std::min)(static_cast<Scalar>(extent.height), std::ceil(static_cast<Scalar>(max_y) + margin));
  if (!(right > left) || !(bottom > top)) {
    return std::nullopt;
  }
  return PixelScissor{
      .x = static_cast<std::int32_t>(left),
      .y = static_cast<std::int32_t>(top),
      .width = static_cast<std::uint32_t>(right - left),
      .height = static_cast<std::uint32_t>(bottom - top),
  };
}

std::uint32_t ceil_div(std::uint32_t value, std::uint32_t divisor) {
  return value / divisor + (value % divisor != 0);
}

} // namespace

BackdropBlurPlan plan_backdrop_blur(
    std::span<const BackdropBlurDraw> backdrops,
    PixelExtent extent) {
  BackdropBlurPlan plan;
  constexpr auto coordinate_limit = std::numeric_limits<std::int32_t>::max();
  if (extent.width > coordinate_limit || extent.height > coordinate_limit) return plan;
  // Float represents every integer through 2^24; preserve the normal path
  // and widen only when an extent can otherwise round outside its boundary.
  const bool wide_extent = extent.width > (1U << 24) || extent.height > (1U << 24);
  for (std::size_t index = 0; index < backdrops.size(); ++index) {
    const BackdropBlurDraw& draw = backdrops[index];
    const float opacity =
        resolve_composed_opacity(draw.metadata, draw.composition_stack);
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
      device[corner] = apply_composed_transform(
          local[corner], draw.metadata, draw.composition_stack);
    }
    const float margin = std::ceil(sigma * 3.0F);
    const std::optional<PixelScissor> region = wide_extent
        ? blur_region<double>(device, margin, extent)
        : blur_region<float>(device, margin, extent);
    if (!region.has_value()) {
      continue;
    }
    const std::uint32_t factor = backdrop_blur_downsample_factor(sigma);
    // The box downsample already has variance (f^2 - 1) / 12 px^2.
    const float box_variance =
        static_cast<float>(factor * factor - 1) / 12.0F;
    const float squared_sigma = sigma * sigma;
    // Preserve ordinary float results; widen only when a finite input's
    // square overflows. Every float sigma can be squared safely in double.
    const float texel_sigma = std::isfinite(squared_sigma)
        ? std::sqrt((std::max)(0.0F, squared_sigma - box_variance)) /
              static_cast<float>(factor)
        : static_cast<float>(std::sqrt(static_cast<double>(sigma) * sigma - box_variance) /
                             static_cast<double>(factor));
    const BorderRadii radii =
        resolve_rounded_rect_radii(rect.size, draw.radius).radii;
    // No tint mixes nothing (Color{} would be opaque black).
    const Color tint = draw.tint.value_or(Color{.a = 0.0F});
    BackdropBlurPass pass{
        .source_index = index,
        .region = *region,
        .factor = factor,
        .texels = {.width = ceil_div(region->width, factor),
                   .height = ceil_div(region->height, factor)},
        .sigma = texel_sigma,
        // Clamp while still floating point: an extreme finite sigma can
        // need more taps than int can represent, even without square overflow.
        .radius = static_cast<int>((std::min)(static_cast<float>(kBackdropBlurMaxRadius),
                                             std::ceil(texel_sigma * 3.0F))),
        .first_vertex = static_cast<std::uint32_t>(plan.vertices.size()),
        .clip_rect =
            resolve_effective_clip_rect(draw.clip_rect, draw.clip_stack),
    };
    for (const std::size_t corner : {0U, 1U, 2U, 2U, 3U, 0U}) {
      plan.vertices.push_back(BackdropBlurVertex{
          .position = {device[corner].x, device[corner].y},
          .local = {local[corner].x, local[corner].y},
          .rect = {rect.origin.x, rect.origin.y,
                   rect.origin.x + rect.size.width,
                   rect.origin.y + rect.size.height},
          .radii = {radii.top_left, radii.top_right, radii.bottom_right,
                    radii.bottom_left},
          .tint = {tint.r, tint.g, tint.b, tint.a},
          .region = {static_cast<float>(region->x),
                     static_cast<float>(region->y),
                     static_cast<float>(factor), opacity},
      });
    }
    plan.passes.push_back(pass);
  }
  return plan;
}

} // namespace cgpui
