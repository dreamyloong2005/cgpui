#include "vulkan_rounded_rect_pipeline_internal.hpp"

namespace cgpui {

VkPushConstantRange vulkan_rounded_rect_push_constant_range() {
  return VkPushConstantRange{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .offset = 0,
      .size = sizeof(VulkanRoundedRectPushConstants),
  };
}

VkPipelineLayoutCreateInfo vulkan_rounded_rect_pipeline_layout_create_info(
    const VkPushConstantRange& push_constant_range) {
  return VkPipelineLayoutCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_constant_range,
  };
}

Result<void> vulkan_create_rounded_rect_pipeline_resources(
    VkDevice device,
    VkRenderPass render_pass,
    VulkanRoundedRectPipelineResources& resources) {
  if (device == VK_NULL_HANDLE || render_pass == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "rounded rectangle pipeline requires device and render pass"));
  }
  vulkan_destroy_rounded_rect_pipeline_resources(device, resources);
  const VkPushConstantRange push_constant_range =
      vulkan_rounded_rect_push_constant_range();
  const VkPipelineLayoutCreateInfo layout_info =
      vulkan_rounded_rect_pipeline_layout_create_info(push_constant_range);
  if (auto result = require_vk_success(
          vkCreatePipelineLayout(
              device, &layout_info, nullptr, &resources.layout),
          "vkCreatePipelineLayout for rounded rectangle pipeline failed");
      !result) {
    return result;
  }
  VulkanRoundedRectShaderModules shader_modules;
  if (auto result =
          vulkan_create_rounded_rect_shader_modules(device, shader_modules);
      !result) {
    vulkan_destroy_rounded_rect_pipeline_resources(device, resources);
    return result;
  }
  const auto shader_stages =
      vulkan_rounded_rect_shader_stage_create_infos(shader_modules);
  const VkVertexInputBindingDescription vertex_binding =
      vulkan_rounded_rect_vertex_binding_description();
  const auto vertex_attributes =
      vulkan_rounded_rect_vertex_attribute_descriptions();
  const VkPipelineVertexInputStateCreateInfo vertex_input{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertex_binding,
      .vertexAttributeDescriptionCount =
          static_cast<std::uint32_t>(vertex_attributes.size()),
      .pVertexAttributeDescriptions = vertex_attributes.data(),
  };
  const VkPipelineInputAssemblyStateCreateInfo input_assembly =
      vulkan_rounded_rect_input_assembly_state();
  const VkPipelineViewportStateCreateInfo viewport =
      vulkan_rounded_rect_viewport_state();
  const VkPipelineRasterizationStateCreateInfo rasterization =
      vulkan_rounded_rect_rasterization_state();
  const VkPipelineMultisampleStateCreateInfo multisample =
      vulkan_rounded_rect_multisample_state();
  const VkPipelineDepthStencilStateCreateInfo depth_stencil =
      vulkan_rounded_rect_depth_stencil_state();
  const VkPipelineColorBlendAttachmentState blend_attachment =
      vulkan_rounded_rect_blend_attachment_state();
  const VkPipelineColorBlendStateCreateInfo color_blend{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &blend_attachment,
  };
  const auto dynamic_states = vulkan_rounded_rect_dynamic_states();
  const VkPipelineDynamicStateCreateInfo dynamic_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount =
          static_cast<std::uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data(),
  };
  const VkGraphicsPipelineCreateInfo pipeline_info{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<std::uint32_t>(shader_stages.size()),
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input,
      .pInputAssemblyState = &input_assembly,
      .pViewportState = &viewport,
      .pRasterizationState = &rasterization,
      .pMultisampleState = &multisample,
      .pDepthStencilState = &depth_stencil,
      .pColorBlendState = &color_blend,
      .pDynamicState = &dynamic_state,
      .layout = resources.layout,
      .renderPass = render_pass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
  };
  const auto pipeline_result = require_vk_success(
      vkCreateGraphicsPipelines(
          device,
          VK_NULL_HANDLE,
          1,
          &pipeline_info,
          nullptr,
          &resources.pipeline),
      "vkCreateGraphicsPipelines for rounded rectangle pipeline failed");
  vulkan_destroy_rounded_rect_shader_modules(device, shader_modules);
  if (!pipeline_result) {
    vulkan_destroy_rounded_rect_pipeline_resources(device, resources);
    return pipeline_result;
  }
  return {};
}

void vulkan_destroy_rounded_rect_pipeline_resources(
    VkDevice device,
    VulkanRoundedRectPipelineResources& resources) {
  if (device == VK_NULL_HANDLE) {
    return;
  }
  if (resources.pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(device, resources.pipeline, nullptr);
  }
  if (resources.layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device, resources.layout, nullptr);
  }
  resources = {};
}

} // namespace cgpui
