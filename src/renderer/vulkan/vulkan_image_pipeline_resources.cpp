#include "vulkan_image_pipeline_resources_internal.hpp"

namespace cgpui {

VkPushConstantRange vulkan_image_push_constant_range() {
  return VkPushConstantRange{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .offset = 0,
      .size = sizeof(VulkanImagePushConstants),
  };
}

VkPipelineLayoutCreateInfo vulkan_image_pipeline_layout_create_info(
    const VkDescriptorSetLayout& descriptor_set_layout,
    const VkPushConstantRange& push_constant_range) {
  return VkPipelineLayoutCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &descriptor_set_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_constant_range,
  };
}

Result<void> vulkan_create_image_pipeline_resources(
    VkDevice device,
    VkRenderPass render_pass,
    VkDescriptorSetLayout image_descriptor_set_layout,
    VulkanImagePipelineResources& resources) {
  if (device == VK_NULL_HANDLE || render_pass == VK_NULL_HANDLE ||
      image_descriptor_set_layout == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image pipeline requires device, render pass, and texture layout"));
  }
  vulkan_destroy_image_pipeline_resources(device, resources);

  const VkPushConstantRange push_constant_range =
      vulkan_image_push_constant_range();
  const VkPipelineLayoutCreateInfo layout_info =
      vulkan_image_pipeline_layout_create_info(
          image_descriptor_set_layout,
          push_constant_range);
  if (auto result = require_vk_success(
          vkCreatePipelineLayout(
              device, &layout_info, nullptr, &resources.layout),
          "vkCreatePipelineLayout for image pipeline failed");
      !result) {
    return result;
  }

  VulkanImageShaderModules shader_modules;
  if (auto result =
          vulkan_create_image_shader_modules(device, shader_modules);
      !result) {
    vulkan_destroy_image_pipeline_resources(device, resources);
    return result;
  }

  const auto shader_stages =
      vulkan_image_shader_stage_create_infos(shader_modules);
  const VkVertexInputBindingDescription vertex_binding =
      vulkan_image_vertex_binding_description();
  const auto vertex_attributes =
      vulkan_image_vertex_attribute_descriptions();
  const VkPipelineVertexInputStateCreateInfo vertex_input{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertex_binding,
      .vertexAttributeDescriptionCount =
          static_cast<std::uint32_t>(vertex_attributes.size()),
      .pVertexAttributeDescriptions = vertex_attributes.data(),
  };
  const VkPipelineInputAssemblyStateCreateInfo input_assembly =
      vulkan_image_pipeline_input_assembly_state();
  const VkPipelineViewportStateCreateInfo viewport =
      vulkan_image_pipeline_viewport_state();
  const VkPipelineRasterizationStateCreateInfo rasterization =
      vulkan_image_pipeline_rasterization_state();
  const VkPipelineMultisampleStateCreateInfo multisample =
      vulkan_image_pipeline_multisample_state();
  const VkPipelineDepthStencilStateCreateInfo depth_stencil =
      vulkan_image_pipeline_depth_stencil_state();
  const VkPipelineColorBlendAttachmentState blend_attachment =
      vulkan_image_pipeline_blend_attachment_state();
  const VkPipelineColorBlendStateCreateInfo color_blend{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &blend_attachment,
  };
  const auto dynamic_states = vulkan_image_pipeline_dynamic_states();
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
      "vkCreateGraphicsPipelines for image pipeline failed");
  vulkan_destroy_image_shader_modules(device, shader_modules);
  if (!pipeline_result) {
    vulkan_destroy_image_pipeline_resources(device, resources);
    return pipeline_result;
  }
  return {};
}

void vulkan_destroy_image_pipeline_resources(
    VkDevice device,
    VulkanImagePipelineResources& resources) {
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
