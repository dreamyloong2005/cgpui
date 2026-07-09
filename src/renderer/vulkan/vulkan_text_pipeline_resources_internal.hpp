#pragma once

#include "vulkan_text_pipeline_internal.hpp"

namespace cgpui {

struct VulkanTextPushConstants {
  std::array<float, 2> framebuffer_size{};
};

struct VulkanTextPipelineResources {
  VkPipelineLayout layout = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;

  [[nodiscard]] bool ready() const {
    return layout != VK_NULL_HANDLE && pipeline != VK_NULL_HANDLE;
  }
};

[[nodiscard]] VkPushConstantRange vulkan_text_push_constant_range();
[[nodiscard]] VkPipelineLayoutCreateInfo
vulkan_text_pipeline_layout_create_info(
    const VkDescriptorSetLayout& descriptor_set_layout,
    const VkPushConstantRange& push_constant_range);
Result<void> vulkan_create_text_pipeline_resources(
    VkDevice device,
    VkRenderPass render_pass,
    VkDescriptorSetLayout glyph_atlas_descriptor_set_layout,
    VulkanTextPipelineResources& resources);
void vulkan_destroy_text_pipeline_resources(
    VkDevice device,
    VulkanTextPipelineResources& resources);

} // namespace cgpui
