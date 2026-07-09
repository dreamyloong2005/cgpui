#pragma once

#include "vulkan_platform_internal.hpp"

#include <array>

namespace cgpui {

struct VulkanTextVertex {
  std::array<float, 2> position{};
  std::array<float, 2> atlas_uv{};
  std::array<float, 4> color{};
};

[[nodiscard]] VkVertexInputBindingDescription
vulkan_text_vertex_binding_description();
[[nodiscard]] std::array<VkVertexInputAttributeDescription, 3>
vulkan_text_vertex_attribute_descriptions();
[[nodiscard]] VkPipelineInputAssemblyStateCreateInfo
vulkan_text_pipeline_input_assembly_state();
[[nodiscard]] VkPipelineViewportStateCreateInfo
vulkan_text_pipeline_viewport_state();
[[nodiscard]] VkPipelineRasterizationStateCreateInfo
vulkan_text_pipeline_rasterization_state();
[[nodiscard]] VkPipelineMultisampleStateCreateInfo
vulkan_text_pipeline_multisample_state();
[[nodiscard]] VkPipelineDepthStencilStateCreateInfo
vulkan_text_pipeline_depth_stencil_state();
[[nodiscard]] VkPipelineColorBlendAttachmentState
vulkan_text_pipeline_blend_attachment_state();
[[nodiscard]] std::array<VkDynamicState, 2>
vulkan_text_pipeline_dynamic_states();

} // namespace cgpui
