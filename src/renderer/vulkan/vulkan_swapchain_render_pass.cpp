#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::create_swapchain_render_pass(
    VulkanSwapchainResources& resources) {
  const VkAttachmentDescription color_attachment{
      .format = resources.format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };
  const VkAttachmentReference color_attachment_reference{
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  const VkSubpassDescription subpass{
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_reference,
  };
  const VkSubpassDependency dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };
  const VkRenderPassCreateInfo render_pass_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &color_attachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency,
  };
  if (auto result = require_vk_success(
          vkCreateRenderPass(
              device_, &render_pass_info, nullptr, &resources.render_pass),
          "vkCreateRenderPass failed");
      !result) {
    return std::unexpected(result.error());
  }
  return {};
}

Result<void> VulkanRendererState::create_swapchain_framebuffers(
    VulkanSwapchainResources& resources) {
  resources.framebuffers.reserve(resources.image_views.size());
  for (VkImageView image_view : resources.image_views) {
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    const VkFramebufferCreateInfo framebuffer_info{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = resources.render_pass,
        .attachmentCount = 1,
        .pAttachments = &image_view,
        .width = resources.extent.width,
        .height = resources.extent.height,
        .layers = 1,
    };
    if (auto result = require_vk_success(
            vkCreateFramebuffer(
                device_, &framebuffer_info, nullptr, &framebuffer),
            "vkCreateFramebuffer failed");
        !result) {
      return std::unexpected(result.error());
    }
    resources.framebuffers.push_back(framebuffer);
  }
  return {};
}

} // namespace cgpui
