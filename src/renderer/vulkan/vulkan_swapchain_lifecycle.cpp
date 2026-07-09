#include "vulkan_internal.hpp"

namespace cgpui {

void VulkanRendererState::destroy_swapchain_resources(
    VulkanSwapchainResources& resources) {
  if (device_ != VK_NULL_HANDLE && command_pool_ != VK_NULL_HANDLE &&
      !resources.command_buffers.empty()) {
    vkFreeCommandBuffers(
        device_,
        command_pool_,
        static_cast<std::uint32_t>(resources.command_buffers.size()),
        resources.command_buffers.data());
  }
  resources.command_buffers.clear();

  for (auto framebuffer : resources.framebuffers) {
    vkDestroyFramebuffer(device_, framebuffer, nullptr);
  }
  resources.framebuffers.clear();

  vulkan_destroy_rounded_rect_pipeline_resources(
      device_, resources.rounded_rect_pipeline);
  vulkan_destroy_text_pipeline_resources(device_, resources.text_pipeline);

  if (resources.render_pass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(device_, resources.render_pass, nullptr);
    resources.render_pass = VK_NULL_HANDLE;
  }

  for (auto image_view : resources.image_views) {
    vkDestroyImageView(device_, image_view, nullptr);
  }
  resources.image_views.clear();
  resources.images.clear();

  if (resources.swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device_, resources.swapchain, nullptr);
    resources.swapchain = VK_NULL_HANDLE;
  }
  resources.format = VK_FORMAT_UNDEFINED;
  resources.extent = VkExtent2D{};
}

void VulkanRendererState::destroy_swapchain() {
  VulkanSwapchainResources resources{
      .swapchain = swapchain_,
      .format = swapchain_format_,
      .extent = swapchain_extent_,
      .images = std::move(swapchain_images_),
      .image_views = std::move(swapchain_image_views_),
      .render_pass = render_pass_,
      .text_pipeline = text_pipeline_resources_,
      .rounded_rect_pipeline = rounded_rect_pipeline_resources_,
      .framebuffers = std::move(framebuffers_),
      .command_buffers = std::move(command_buffers_),
  };
  swapchain_ = VK_NULL_HANDLE;
  swapchain_format_ = VK_FORMAT_UNDEFINED;
  swapchain_extent_ = VkExtent2D{};
  render_pass_ = VK_NULL_HANDLE;
  text_pipeline_resources_ = {};
  rounded_rect_pipeline_resources_ = {};
  destroy_swapchain_resources(resources);
}

void VulkanRendererState::install_swapchain(
    VulkanSwapchainResources&& resources) {
  swapchain_ = resources.swapchain;
  swapchain_format_ = resources.format;
  swapchain_extent_ = resources.extent;
  swapchain_images_ = std::move(resources.images);
  swapchain_image_views_ = std::move(resources.image_views);
  render_pass_ = resources.render_pass;
  text_pipeline_resources_ = resources.text_pipeline;
  rounded_rect_pipeline_resources_ = resources.rounded_rect_pipeline;
  framebuffers_ = std::move(resources.framebuffers);
  command_buffers_ = std::move(resources.command_buffers);
  resources.swapchain = VK_NULL_HANDLE;
  resources.render_pass = VK_NULL_HANDLE;
  resources.text_pipeline = {};
  resources.rounded_rect_pipeline = {};
  resources.format = VK_FORMAT_UNDEFINED;
  resources.extent = VkExtent2D{};
}

Result<void> VulkanRendererState::create_swapchain() {
  auto resources = create_swapchain_resources(VK_NULL_HANDLE);
  if (!resources) {
    return std::unexpected(resources.error());
  }

  install_swapchain(std::move(*resources));
  return {};
}

} // namespace cgpui
