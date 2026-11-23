#include "vulkan_backdrop_blur_plan_internal.hpp"
#include "cgpui/renderer/planning/backdrop_blur_plan.hpp"

#include <utility>

namespace cgpui {

std::uint32_t vulkan_backdrop_blur_downsample_factor(float sigma) {
  return backdrop_blur_downsample_factor(sigma);
}

VulkanBackdropBlurPlan vulkan_plan_backdrop_blur(
    std::span<const BackdropBlurDraw> backdrops, VkExtent2D extent) {
  auto neutral = plan_backdrop_blur(backdrops, PixelExtent{extent.width, extent.height});
  VulkanBackdropBlurPlan result;
  result.vertices = std::move(neutral.vertices);
  result.passes.reserve(neutral.passes.size());
  for (const auto& pass : neutral.passes) {
    result.passes.push_back(VulkanBackdropBlurPass{
        .source_index = pass.source_index,
        .region = {.offset = {pass.region.x, pass.region.y},
                   .extent = {pass.region.width, pass.region.height}},
        .factor = pass.factor,
        .texels = {pass.texels.width, pass.texels.height},
        .sigma = pass.sigma,
        .radius = pass.radius,
        .first_vertex = pass.first_vertex,
        .clip_rect = pass.clip_rect,
    });
  }
  return result;
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
