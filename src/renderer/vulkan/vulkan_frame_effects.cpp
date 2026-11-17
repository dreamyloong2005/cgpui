#include "vulkan_internal.hpp"

namespace cgpui {

// Effect resources are created on the first frame that needs them. Frames
// without shadows upload nothing (empty geometry keeps the buffers); frames
// without backdrops clear the plan and touch no offscreen resource.
Result<void> VulkanRendererState::prepare_effect_frame(
    std::span<const BoxShadowDraw> box_shadows,
    std::span<const BackdropBlurDraw> backdrops) {
  if (!box_shadows.empty() && !box_shadow_pipeline_resources_.ready()) {
    if (auto result = vulkan_create_box_shadow_pipeline_resources(
            device_, render_pass_, box_shadow_pipeline_resources_);
        !result) {
      return result;
    }
  }
  if (auto result = vulkan_upload_box_shadow_buffers(
          physical_device_, device_, box_shadows, box_shadow_buffers_);
      !result) {
    return result;
  }
  backdrop_blur_plan_.passes.clear();
  backdrop_blur_plan_.vertices.clear();
  if (backdrops.empty()) {
    return {};
  }
  backdrop_blur_plan_ = vulkan_plan_backdrop_blur(backdrops, swapchain_extent_);
  if (backdrop_blur_plan_.passes.empty()) {
    return {};
  }
  if (auto result = vulkan_prepare_backdrop_blur_resources(
          physical_device_, device_, swapchain_format_, swapchain_extent_,
          backdrop_blur_resources_);
      !result) {
    return result;
  }
  return vulkan_upload_frame_geometry_buffer(
      physical_device_, device_, backdrop_blur_plan_.vertices.data(),
      backdrop_blur_plan_.vertices.size() * sizeof(VulkanBackdropBlurVertex),
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, backdrop_blur_resources_.vertices);
}

VulkanFrameEffects VulkanRendererState::frame_effects() const {
  return VulkanFrameEffects{
      .box_shadow = {.pipeline = &box_shadow_pipeline_resources_,
                     .buffers = &box_shadow_buffers_},
      .backdrop_blur = {.resources = &backdrop_blur_resources_,
                        .passes = backdrop_blur_plan_.passes},
  };
}

RendererBackdropBlurDiagnostics
VulkanRendererState::backdrop_blur_diagnostics() const {
  const bool active = frame_effects().backdrop_blur.active();
  const std::size_t count = active ? backdrop_blur_plan_.passes.size() : 0;
  return RendererBackdropBlurDiagnostics{
      .blurred_backdrop_count = count,
      .offscreen_pass_count = vulkan_backdrop_blur_offscreen_pass_count(count),
      .offscreen_targets_allocated = backdrop_blur_resources_.ready(),
  };
}

RendererEffectSupport VulkanRendererState::backdrop_blur_support() const {
  if (physical_device_ == VK_NULL_HANDLE ||
      swapchain_format_ == VK_FORMAT_UNDEFINED) {
    return {.supported = false, .reason = "Vulkan swapchain is not ready"};
  }
  VkFormatProperties properties{};
  vkGetPhysicalDeviceFormatProperties(
      physical_device_, swapchain_format_, &properties);
  return vulkan_backdrop_blur_support(properties.optimalTilingFeatures);
}

} // namespace cgpui
