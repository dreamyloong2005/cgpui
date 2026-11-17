#include "vulkan_backdrop_blur_resources_internal.hpp"

#include "vulkan_device_internal.hpp"

namespace cgpui {
namespace {

// Offscreen passes must stay compatible with the swapchain render pass so
// every pipeline draws into both: same single attachment format and the
// same single dependency. They end in COLOR_ATTACHMENT_OPTIMAL; recording
// transitions each target to SHADER_READ_ONLY_OPTIMAL with a barrier.
Result<VkRenderPass> create_pass(
    VkDevice device,
    VkFormat format,
    VkAttachmentLoadOp load,
    VkImageLayout initial) {
  const VkAttachmentDescription attachment{
      .format = format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = load,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = initial,
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  const VkAttachmentReference reference{
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
  const VkSubpassDescription subpass{
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &reference,
  };
  // Identical to vulkan_swapchain_render_pass.cpp. Color output waits on
  // earlier color output, which also orders earlier fragment shader reads.
  const std::array dependencies{
      VkSubpassDependency{
          .srcSubpass = VK_SUBPASS_EXTERNAL,
          .dstSubpass = 0,
          .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          .srcAccessMask = 0,
          .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      },
  };
  const VkRenderPassCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &attachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = static_cast<std::uint32_t>(dependencies.size()),
      .pDependencies = dependencies.data(),
  };
  VkRenderPass pass = VK_NULL_HANDLE;
  if (auto result = require_vk_success(
          vkCreateRenderPass(device, &info, nullptr, &pass),
          "vkCreateRenderPass for backdrop blur failed");
      !result) {
    return std::unexpected(result.error());
  }
  return pass;
}

Result<void> create_target(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const VulkanBackdropBlurResources& resources,
    VulkanBackdropBlurTarget& target) {
  const VkImageCreateInfo image_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = resources.format,
      .extent = {.width = resources.extent.width,
                 .height = resources.extent.height,
                 .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  if (auto result = require_vk_success(
          vkCreateImage(device, &image_info, nullptr, &target.image),
          "vkCreateImage for backdrop blur target failed");
      !result) {
    return result;
  }
  VkMemoryRequirements requirements{};
  vkGetImageMemoryRequirements(device, target.image, &requirements);
  auto memory_type = vulkan_find_memory_type(
      physical_device, requirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (!memory_type) {
    return std::unexpected(memory_type.error());
  }
  const VkMemoryAllocateInfo allocation{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = requirements.size,
      .memoryTypeIndex = *memory_type,
  };
  if (auto result = require_vk_success(
          vkAllocateMemory(device, &allocation, nullptr, &target.memory),
          "vkAllocateMemory for backdrop blur target failed");
      !result) {
    return result;
  }
  vkBindImageMemory(device, target.image, target.memory, 0);
  const VkImageViewCreateInfo view_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = target.image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = resources.format,
      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                           .levelCount = 1,
                           .layerCount = 1},
  };
  if (auto result = require_vk_success(
          vkCreateImageView(device, &view_info, nullptr, &target.view),
          "vkCreateImageView for backdrop blur target failed");
      !result) {
    return result;
  }
  const VkFramebufferCreateInfo framebuffer_info{
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = resources.scene_load_pass,
      .attachmentCount = 1,
      .pAttachments = &target.view,
      .width = resources.extent.width,
      .height = resources.extent.height,
      .layers = 1,
  };
  return require_vk_success(
      vkCreateFramebuffer(
          device, &framebuffer_info, nullptr, &target.framebuffer),
      "vkCreateFramebuffer for backdrop blur target failed");
}

} // namespace

Result<void> vulkan_create_backdrop_blur_targets(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkFormat format,
    VkExtent2D extent,
    VulkanBackdropBlurResources& resources) {
  resources.format = format;
  resources.extent = extent;
  auto clear = create_pass(
      device, format, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED);
  if (!clear) return std::unexpected(clear.error());
  resources.scene_clear_pass = *clear;
  auto load = create_pass(
      device, format, VK_ATTACHMENT_LOAD_OP_LOAD,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  if (!load) return std::unexpected(load.error());
  resources.scene_load_pass = *load;
  auto blur = create_pass(
      device, format, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED);
  if (!blur) return std::unexpected(blur.error());
  resources.blur_pass = *blur;
  for (VulkanBackdropBlurTarget& target : resources.targets) {
    if (auto result =
            create_target(physical_device, device, resources, target);
        !result) {
      return result;
    }
  }
  return {};
}

} // namespace cgpui
