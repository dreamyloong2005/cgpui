#include "vulkan_internal.hpp"

namespace cgpui {

Result<VulkanSwapchainResources> VulkanRendererState::create_swapchain_resources(
    VkSwapchainKHR old_swapchain,
    bool* retired_old_swapchain) {
  if (retired_old_swapchain != nullptr) {
    *retired_old_swapchain = false;
  }

  auto details = query_vulkan_swapchain_surface(physical_device_, surface_);
  if (!details) {
    return std::unexpected(details.error());
  }

  const VulkanSwapchainCreatePlan plan =
      build_vulkan_swapchain_create_plan(
          *details,
          descriptor_.framebuffer_size,
          graphics_queue_family_,
          present_queue_family_);
  const VkSwapchainCreateInfoKHR create_info =
      make_vulkan_swapchain_create_info(plan, surface_, old_swapchain);

  VulkanSwapchainResources resources{
      .format = plan.surface_format.format,
      .extent = plan.extent,
  };
  if (auto result = require_vk_success(
          vkCreateSwapchainKHR(
              device_, &create_info, nullptr, &resources.swapchain),
          "vkCreateSwapchainKHR failed");
      !result) {
    return std::unexpected(result.error());
  }
  if (retired_old_swapchain != nullptr && old_swapchain != VK_NULL_HANDLE) {
    *retired_old_swapchain = true;
  }

  if (auto result = populate_swapchain_images(resources); !result) {
    destroy_swapchain_resources(resources);
    return std::unexpected(result.error());
  }
  if (auto result = create_swapchain_image_views(resources); !result) {
    destroy_swapchain_resources(resources);
    return std::unexpected(result.error());
  }
  if (auto result = create_swapchain_render_pass(resources); !result) {
    destroy_swapchain_resources(resources);
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_create_text_pipeline_resources(
          device_,
          resources.render_pass,
          glyph_atlas_resources_.descriptor_set_layout,
          resources.text_pipeline);
      !result) {
    destroy_swapchain_resources(resources);
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_create_image_pipeline_resources(
          device_,
          resources.render_pass,
          image_texture_resources_.descriptor_set_layout,
          resources.image_pipeline);
      !result) {
    destroy_swapchain_resources(resources);
    return std::unexpected(result.error());
  }
  if (auto result = vulkan_create_rounded_rect_pipeline_resources(
          device_,
          resources.render_pass,
          resources.rounded_rect_pipeline);
      !result) {
    destroy_swapchain_resources(resources);
    return std::unexpected(result.error());
  }
  if (auto result = create_swapchain_framebuffers(resources); !result) {
    destroy_swapchain_resources(resources);
    return std::unexpected(result.error());
  }
  if (auto result = allocate_swapchain_command_buffers(resources); !result) {
    destroy_swapchain_resources(resources);
    return std::unexpected(result.error());
  }

  return resources;
}

} // namespace cgpui
