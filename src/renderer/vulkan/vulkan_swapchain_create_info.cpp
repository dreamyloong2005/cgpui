#include "vulkan_internal.hpp"

namespace cgpui {

VulkanSwapchainCreatePlan build_vulkan_swapchain_create_plan(
    const VulkanSwapchainSurfaceDetails& details,
    Size framebuffer_size,
    bool transparent_background,
    std::uint32_t graphics_queue_family,
    std::uint32_t present_queue_family) {
  const VulkanPresentPacingPlan present_pacing = vulkan_plan_present_pacing(
      details.present_modes,
      details.capabilities.minImageCount,
      details.capabilities.maxImageCount);
  VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if ((details.capabilities.supportedUsageFlags &
       VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0) {
    image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }
  return VulkanSwapchainCreatePlan{
      .surface_format = choose_vulkan_surface_format(details.formats),
      .present_pacing = present_pacing,
      .extent = choose_vulkan_extent(details.capabilities, framebuffer_size),
      .image_usage = image_usage,
      .composite_alpha =
          choose_vulkan_composite_alpha(
              details.capabilities.supportedCompositeAlpha,
              transparent_background),
      .pre_transform = details.capabilities.currentTransform,
      .queue_family_indices = {graphics_queue_family, present_queue_family},
      .separate_queue_families = graphics_queue_family != present_queue_family,
  };
}

VkSwapchainCreateInfoKHR make_vulkan_swapchain_create_info(
    const VulkanSwapchainCreatePlan& plan,
    VkSurfaceKHR surface,
    VkSwapchainKHR old_swapchain) {
  return VkSwapchainCreateInfoKHR{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = plan.present_pacing.swapchain_image_count,
      .imageFormat = plan.surface_format.format,
      .imageColorSpace = plan.surface_format.colorSpace,
      .imageExtent = plan.extent,
      .imageArrayLayers = 1,
      .imageUsage = plan.image_usage,
      .imageSharingMode = plan.separate_queue_families
          ? VK_SHARING_MODE_CONCURRENT
          : VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = plan.separate_queue_families
          ? static_cast<std::uint32_t>(plan.queue_family_indices.size())
          : 0U,
      .pQueueFamilyIndices = plan.separate_queue_families
          ? plan.queue_family_indices.data()
          : nullptr,
      .preTransform = plan.pre_transform,
      .compositeAlpha = plan.composite_alpha,
      .presentMode = plan.present_pacing.present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = old_swapchain,
  };
}

} // namespace cgpui
