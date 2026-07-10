#pragma once

#include "vulkan_image_pipeline_resources_internal.hpp"
#include "vulkan_platform_internal.hpp"
#include "vulkan_present_pacing_internal.hpp"
#include "vulkan_rounded_rect_pipeline_internal.hpp"
#include "vulkan_text_pipeline_resources_internal.hpp"

namespace cgpui {

struct VulkanSwapchainResources {
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkFormat format = VK_FORMAT_UNDEFINED;
  VkExtent2D extent{};
  VkImageUsageFlags image_usage = 0;
  VulkanPresentPacingPlan present_pacing{};
  std::vector<VkImage> images;
  std::vector<VkImageView> image_views;
  VkRenderPass render_pass = VK_NULL_HANDLE;
  VulkanImagePipelineResources image_pipeline;
  VulkanTextPipelineResources text_pipeline;
  VulkanRoundedRectPipelineResources rounded_rect_pipeline;
  std::vector<VkFramebuffer> framebuffers;
  std::vector<VkCommandBuffer> command_buffers;
};

struct VulkanSwapchainSurfaceDetails {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

struct VulkanSwapchainCreatePlan {
  VkSurfaceFormatKHR surface_format{};
  VulkanPresentPacingPlan present_pacing{};
  VkExtent2D extent{};
  VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  VkCompositeAlphaFlagBitsKHR composite_alpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  VkSurfaceTransformFlagBitsKHR pre_transform =
      VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  std::array<std::uint32_t, 2> queue_family_indices{};
  bool separate_queue_families = false;
};

Result<VulkanSwapchainSurfaceDetails> query_vulkan_swapchain_surface(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface);
VulkanSwapchainCreatePlan build_vulkan_swapchain_create_plan(
    const VulkanSwapchainSurfaceDetails& details,
    Size framebuffer_size,
    std::uint32_t graphics_queue_family,
    std::uint32_t present_queue_family);
VkSwapchainCreateInfoKHR make_vulkan_swapchain_create_info(
    const VulkanSwapchainCreatePlan& plan,
    VkSurfaceKHR surface,
    VkSwapchainKHR old_swapchain);

} // namespace cgpui
