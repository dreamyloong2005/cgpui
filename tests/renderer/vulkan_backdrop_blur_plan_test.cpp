#include "vulkan_backdrop_blur_plan_internal.hpp"
#include "vulkan_device_pixels_internal.hpp"

#include <cmath>
#include <vector>

// Backdrop blur planning without a GPU: regions, downsampling, kernel,
// support check and the offscreen pass count the renderer reports.
namespace {

bool close_to(float a, float b) { return std::abs(a - b) < 0.01F; }

cgpui::BackdropBlurDraw draw(cgpui::Rect rect, float blur) {
  return cgpui::BackdropBlurDraw{
      .rect = rect,
      .radius = cgpui::BorderRadii::all(8.0F),
      .blur_radius = blur,
      .tint = cgpui::Color{.r = 1.0F, .a = 0.25F},
      .metadata = {.opacity = 0.5F},
  };
}

int test_region_and_kernel() {
  const std::vector draws{draw({.origin = {20, 30}, .size = {100, 50}}, 10.0F)};
  const auto plan =
      cgpui::vulkan_plan_backdrop_blur(draws, VkExtent2D{.width = 200, .height = 100});
  if (plan.passes.size() != 1 || plan.vertices.size() != 6) return 1;
  const auto& pass = plan.passes.front();
  // Quad (rect + 1 px) grown by ceil(3 sigma) = 30, clamped to the 200 x 100 frame.
  if (pass.region.offset.x != 0 || pass.region.offset.y != 0 ||
      pass.region.extent.width != 151 || pass.region.extent.height != 100 ||
      pass.factor != 2 || pass.texels.width != 76 || pass.texels.height != 50) {
    return 2;
  }
  // sigma' = sqrt(100 - 3/12) / 2; radius = ceil(3 sigma').
  if (!close_to(pass.sigma, std::sqrt(99.75F) / 2.0F) || pass.radius != 15) return 3;
  const auto& vertex = plan.vertices.front();
  return close_to(vertex.local[0], 19.0F) && close_to(vertex.rect[2], 120.0F) &&
                 close_to(vertex.radii[0], 8.0F) && close_to(vertex.tint[3], 0.25F) &&
                 close_to(vertex.region[2], 2.0F) && close_to(vertex.region[3], 0.5F)
             ? 0
             : 4;
}

int test_skips_and_factors() {
  const std::vector draws{
      draw({.origin = {300, 300}, .size = {10, 10}}, 2.0F),
      draw({.origin = {0, 0}, .size = {0, 10}}, 2.0F),
      draw({.origin = {0, 0}, .size = {10, 10}}, 2.0F),
  };
  const auto plan = cgpui::vulkan_plan_backdrop_blur(
      draws, VkExtent2D{.width = 64, .height = 64});
  if (plan.passes.size() != 1 || plan.passes.front().source_index != 2 ||
      plan.passes.front().first_vertex != 0) {
    return 10;
  }
  return cgpui::vulkan_backdrop_blur_downsample_factor(4.0F) == 1 &&
                 cgpui::vulkan_backdrop_blur_downsample_factor(16.0F) == 2 &&
                 cgpui::vulkan_backdrop_blur_downsample_factor(40.0F) == 4 &&
                 cgpui::vulkan_backdrop_blur_downsample_factor(100.0F) == 8 &&
                 cgpui::vulkan_backdrop_blur_offscreen_pass_count(0) == 0 &&
                 cgpui::vulkan_backdrop_blur_offscreen_pass_count(2) == 9
             ? 0
             : 11;
}

int test_support_and_scale() {
  const auto missing = cgpui::vulkan_backdrop_blur_support(
      VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
  const auto present = cgpui::vulkan_backdrop_blur_support(
      VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
      VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
  if (missing.supported || missing.reason.empty() || !present.supported) {
    return 20;
  }
  const auto device = cgpui::vulkan_to_device_pixels(
      draw({.origin = {10, 10}, .size = {20, 20}}, 6.0F), cgpui::DpiScale{1.5F});
  return close_to(device.blur_radius, 9.0F) && close_to(device.rect.origin.x, 15.0F) &&
                 close_to(device.radius.top_left, 12.0F)
             ? 0
             : 21;
}

} // namespace

int main() {
  for (const int result :
       {test_region_and_kernel(), test_skips_and_factors(), test_support_and_scale()}) {
    if (result != 0) return result;
  }
  return 0;
}
