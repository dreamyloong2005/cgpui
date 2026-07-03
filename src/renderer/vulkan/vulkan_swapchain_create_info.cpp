#include "vulkan_internal.hpp"

namespace cgpui {

VulkanSwapchainCreatePlan build_vulkan_swapchain_create_plan(
    const VulkanSwapchainSurfaceDetails& details,
    Size framebuffer_size,
    std::uint32_t graphics_queue_family,
    std::uint32_t present_queue_family) {
  VulkanSwapchainCreatePlan plan{
      .surface_format = choose_vulkan_surface_format(details.formats),
      .present_mode = choose_vulkan_present_mode(details.present_modes),
      .extent = choose_vulkan_extent(details.capabilities, framebuffer_size),
      .composite_alpha =
          choose_vulkan_composite_alpha(details.capabilities.supportedCompositeAlpha),
      .pre_transform = details.capabilities.currentTransform,
      .image_count = details.capabilities.minImageCount + 1,
      .queue_family_indices = {graphics_queue_family, present_queue_family},
      .separate_queue_families = graphics_queue_family != present_queue_family,
  };
  if (details.capabilities.maxImageCount != 0 &&
      plan.image_count > details.capabilities.maxImageCount) {
    plan.image_count = details.capabilities.maxImageCount;
  }
  return plan;
}

VkSwapchainCreateInfoKHR make_vulkan_swapchain_create_info(
    const VulkanSwapchainCreatePlan& plan,
    VkSurfaceKHR surface,
    VkSwapchainKHR old_swapchain) {
  return VkSwapchainCreateInfoKHR{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = plan.image_count,
      .imageFormat = plan.surface_format.format,
      .imageColorSpace = plan.surface_format.colorSpace,
      .imageExtent = plan.extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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
      .presentMode = plan.present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = old_swapchain,
  };
}

} // namespace cgpui
