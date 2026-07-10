#include "vulkan_image_pipeline_internal.hpp"

#include <cstddef>

namespace cgpui {

VkVertexInputBindingDescription vulkan_image_vertex_binding_description() {
  return VkVertexInputBindingDescription{
      .binding = 0,
      .stride = sizeof(VulkanImageVertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
}

std::array<VkVertexInputAttributeDescription, 3>
vulkan_image_vertex_attribute_descriptions() {
  return {
      VkVertexInputAttributeDescription{
          .location = 0,
          .binding = 0,
          .format = VK_FORMAT_R32G32_SFLOAT,
          .offset = offsetof(VulkanImageVertex, position),
      },
      VkVertexInputAttributeDescription{
          .location = 1,
          .binding = 0,
          .format = VK_FORMAT_R32G32_SFLOAT,
          .offset = offsetof(VulkanImageVertex, image_uv),
      },
      VkVertexInputAttributeDescription{
          .location = 2,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .offset = offsetof(VulkanImageVertex, color),
      },
  };
}

VkPipelineInputAssemblyStateCreateInfo
vulkan_image_pipeline_input_assembly_state() {
  return VkPipelineInputAssemblyStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };
}

VkPipelineViewportStateCreateInfo vulkan_image_pipeline_viewport_state() {
  return VkPipelineViewportStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };
}

VkPipelineRasterizationStateCreateInfo
vulkan_image_pipeline_rasterization_state() {
  return VkPipelineRasterizationStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0F,
  };
}

VkPipelineMultisampleStateCreateInfo
vulkan_image_pipeline_multisample_state() {
  return VkPipelineMultisampleStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
  };
}

VkPipelineDepthStencilStateCreateInfo
vulkan_image_pipeline_depth_stencil_state() {
  return VkPipelineDepthStencilStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_FALSE,
      .depthWriteEnable = VK_FALSE,
      .depthCompareOp = VK_COMPARE_OP_ALWAYS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
  };
}

VkPipelineColorBlendAttachmentState
vulkan_image_pipeline_blend_attachment_state() {
  return VkPipelineColorBlendAttachmentState{
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask =
          VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };
}

std::array<VkDynamicState, 2> vulkan_image_pipeline_dynamic_states() {
  return {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };
}

} // namespace cgpui
