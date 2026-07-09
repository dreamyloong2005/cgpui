#pragma once

#include "vulkan_platform_internal.hpp"
#include "vulkan_rounded_rect_geometry_internal.hpp"

#include <array>

namespace cgpui {

struct VulkanRoundedRectPushConstants {
  std::array<float, 2> framebuffer_size{};
};

struct VulkanRoundedRectShaderModules {
  VkShaderModule vertex = VK_NULL_HANDLE;
  VkShaderModule fragment = VK_NULL_HANDLE;

  [[nodiscard]] bool ready() const {
    return vertex != VK_NULL_HANDLE && fragment != VK_NULL_HANDLE;
  }
};

struct VulkanRoundedRectPipelineResources {
  VkPipelineLayout layout = VK_NULL_HANDLE;
  VkPipeline pipeline = VK_NULL_HANDLE;

  [[nodiscard]] bool ready() const {
    return layout != VK_NULL_HANDLE && pipeline != VK_NULL_HANDLE;
  }
};

[[nodiscard]] std::span<const std::uint32_t>
vulkan_rounded_rect_vertex_shader_spirv();
[[nodiscard]] std::span<const std::uint32_t>
vulkan_rounded_rect_fragment_shader_spirv();
[[nodiscard]] VkShaderModuleCreateInfo
vulkan_rounded_rect_shader_module_create_info(
    std::span<const std::uint32_t> spirv);
Result<void> vulkan_create_rounded_rect_shader_modules(
    VkDevice device,
    VulkanRoundedRectShaderModules& modules);
void vulkan_destroy_rounded_rect_shader_modules(
    VkDevice device,
    VulkanRoundedRectShaderModules& modules);
[[nodiscard]] std::array<VkPipelineShaderStageCreateInfo, 2>
vulkan_rounded_rect_shader_stage_create_infos(
    const VulkanRoundedRectShaderModules& modules);

[[nodiscard]] VkVertexInputBindingDescription
vulkan_rounded_rect_vertex_binding_description();
[[nodiscard]] std::array<VkVertexInputAttributeDescription, 3>
vulkan_rounded_rect_vertex_attribute_descriptions();
[[nodiscard]] VkPipelineInputAssemblyStateCreateInfo
vulkan_rounded_rect_input_assembly_state();
[[nodiscard]] VkPipelineViewportStateCreateInfo
vulkan_rounded_rect_viewport_state();
[[nodiscard]] VkPipelineRasterizationStateCreateInfo
vulkan_rounded_rect_rasterization_state();
[[nodiscard]] VkPipelineMultisampleStateCreateInfo
vulkan_rounded_rect_multisample_state();
[[nodiscard]] VkPipelineDepthStencilStateCreateInfo
vulkan_rounded_rect_depth_stencil_state();
[[nodiscard]] VkPipelineColorBlendAttachmentState
vulkan_rounded_rect_blend_attachment_state();
[[nodiscard]] std::array<VkDynamicState, 2>
vulkan_rounded_rect_dynamic_states();

[[nodiscard]] VkPushConstantRange vulkan_rounded_rect_push_constant_range();
[[nodiscard]] VkPipelineLayoutCreateInfo
vulkan_rounded_rect_pipeline_layout_create_info(
    const VkPushConstantRange& push_constant_range);
Result<void> vulkan_create_rounded_rect_pipeline_resources(
    VkDevice device,
    VkRenderPass render_pass,
    VulkanRoundedRectPipelineResources& resources);
void vulkan_destroy_rounded_rect_pipeline_resources(
    VkDevice device,
    VulkanRoundedRectPipelineResources& resources);

} // namespace cgpui
