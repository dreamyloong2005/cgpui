#pragma once

#include "vulkan_image_pipeline_internal.hpp"

namespace cgpui {

struct VulkanImagePushConstants {
  std::array<float, 2> framebuffer_size{};
};

struct VulkanImagePipelineResources {
  VkPipelineLayout layout = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;

  [[nodiscard]] bool ready() const {
    return layout != VK_NULL_HANDLE && pipeline != VK_NULL_HANDLE;
  }
};

[[nodiscard]] VkPushConstantRange vulkan_image_push_constant_range();
[[nodiscard]] VkPipelineLayoutCreateInfo
vulkan_image_pipeline_layout_create_info(
    const VkDescriptorSetLayout& descriptor_set_layout,
    const VkPushConstantRange& push_constant_range);
Result<void> vulkan_create_image_pipeline_resources(
    VkDevice device,
    VkRenderPass render_pass,
    VkDescriptorSetLayout image_descriptor_set_layout,
    VulkanImagePipelineResources& resources);
void vulkan_destroy_image_pipeline_resources(
    VkDevice device,
    VulkanImagePipelineResources& resources);

} // namespace cgpui
