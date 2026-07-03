#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::populate_swapchain_images(
    VulkanSwapchainResources& resources) {
  std::uint32_t swapchain_image_count = 0;
  if (auto result = require_vk_success(
          vkGetSwapchainImagesKHR(
              device_, resources.swapchain, &swapchain_image_count, nullptr),
          "vkGetSwapchainImagesKHR failed");
      !result) {
    return std::unexpected(result.error());
  }
  resources.images.resize(swapchain_image_count);
  if (auto result = require_vk_success(
          vkGetSwapchainImagesKHR(
              device_,
              resources.swapchain,
              &swapchain_image_count,
              resources.images.data()),
          "vkGetSwapchainImagesKHR failed");
      !result) {
    return std::unexpected(result.error());
  }
  return {};
}

Result<void> VulkanRendererState::create_swapchain_image_views(
    VulkanSwapchainResources& resources) {
  resources.image_views.reserve(resources.images.size());
  for (VkImage image : resources.images) {
    VkImageView image_view = VK_NULL_HANDLE;
    const VkImageViewCreateInfo image_view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = resources.format,
        .components =
            VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    if (auto result = require_vk_success(
            vkCreateImageView(device_, &image_view_info, nullptr, &image_view),
            "vkCreateImageView failed");
        !result) {
      return std::unexpected(result.error());
    }
    resources.image_views.push_back(image_view);
  }
  return {};
}

} // namespace cgpui
