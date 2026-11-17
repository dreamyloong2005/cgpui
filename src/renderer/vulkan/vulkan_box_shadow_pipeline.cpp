#include "vulkan_box_shadow_pipeline_internal.hpp"

namespace cgpui {
namespace {

Result<VkShaderModule> create_module(
    VkDevice device,
    std::span<const std::uint32_t> spirv) {
  VkShaderModule module = VK_NULL_HANDLE;
  const VkShaderModuleCreateInfo info =
      vulkan_rounded_rect_shader_module_create_info(spirv);
  if (auto result = require_vk_success(
          vkCreateShaderModule(device, &info, nullptr, &module),
          "vkCreateShaderModule for box shadow shader failed");
      !result) {
    return std::unexpected(result.error());
  }
  return module;
}

Result<void> create_pipeline(
    VkDevice device,
    VkRenderPass render_pass,
    const VulkanRoundedRectShaderModules& modules,
    VulkanRoundedRectPipelineResources& resources) {
  const auto stages = vulkan_rounded_rect_shader_stage_create_infos(modules);
  const VkVertexInputBindingDescription binding =
      vulkan_box_shadow_vertex_binding_description();
  const auto attributes = vulkan_box_shadow_vertex_attribute_descriptions();
  const VkPipelineVertexInputStateCreateInfo vertex_input{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &binding,
      .vertexAttributeDescriptionCount =
          static_cast<std::uint32_t>(attributes.size()),
      .pVertexAttributeDescriptions = attributes.data(),
  };
  const auto input_assembly = vulkan_rounded_rect_input_assembly_state();
  const auto viewport = vulkan_rounded_rect_viewport_state();
  const auto rasterization = vulkan_rounded_rect_rasterization_state();
  const auto multisample = vulkan_rounded_rect_multisample_state();
  const auto depth_stencil = vulkan_rounded_rect_depth_stencil_state();
  const auto blend_attachment = vulkan_rounded_rect_blend_attachment_state();
  const VkPipelineColorBlendStateCreateInfo color_blend{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &blend_attachment,
  };
  const auto dynamic_states = vulkan_rounded_rect_dynamic_states();
  const VkPipelineDynamicStateCreateInfo dynamic_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data(),
  };
  const VkGraphicsPipelineCreateInfo pipeline_info{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<std::uint32_t>(stages.size()),
      .pStages = stages.data(),
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
  return require_vk_success(
      vkCreateGraphicsPipelines(
          device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
          &resources.pipeline),
      "vkCreateGraphicsPipelines for box shadow pipeline failed");
}

} // namespace

Result<void> vulkan_create_box_shadow_pipeline_resources(
    VkDevice device,
    VkRenderPass render_pass,
    VulkanRoundedRectPipelineResources& resources) {
  if (device == VK_NULL_HANDLE || render_pass == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "box shadow pipeline requires device and render pass"));
  }
  vulkan_destroy_rounded_rect_pipeline_resources(device, resources);
  const VkPushConstantRange push_range =
      vulkan_rounded_rect_push_constant_range();
  const VkPipelineLayoutCreateInfo layout_info =
      vulkan_rounded_rect_pipeline_layout_create_info(push_range);
  if (auto result = require_vk_success(
          vkCreatePipelineLayout(
              device, &layout_info, nullptr, &resources.layout),
          "vkCreatePipelineLayout for box shadow pipeline failed");
      !result) {
    return result;
  }
  VulkanRoundedRectShaderModules modules;
  auto vertex = create_module(device, vulkan_box_shadow_vertex_shader_spirv());
  auto fragment =
      create_module(device, vulkan_box_shadow_fragment_shader_spirv());
  modules.vertex = vertex ? *vertex : VK_NULL_HANDLE;
  modules.fragment = fragment ? *fragment : VK_NULL_HANDLE;
  Result<void> result{};
  if (!vertex) {
    result = std::unexpected(vertex.error());
  } else if (!fragment) {
    result = std::unexpected(fragment.error());
  } else {
    result = create_pipeline(device, render_pass, modules, resources);
  }
  if (modules.vertex != VK_NULL_HANDLE) {
    vkDestroyShaderModule(device, modules.vertex, nullptr);
  }
  if (modules.fragment != VK_NULL_HANDLE) {
    vkDestroyShaderModule(device, modules.fragment, nullptr);
  }
  if (!result) {
    vulkan_destroy_rounded_rect_pipeline_resources(device, resources);
  }
  return result;
}

} // namespace cgpui
