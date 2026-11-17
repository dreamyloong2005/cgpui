#include "vulkan_backdrop_blur_resources_internal.hpp"

#include "vulkan_rounded_rect_pipeline_internal.hpp"

#include <cstddef>
#include <utility>

namespace cgpui {
namespace {

// Indices into vulkan_backdrop_blur_shader_spirv().
constexpr std::size_t fullscreen_vertex = 0;
constexpr std::size_t downsample_fragment = 1;
constexpr std::size_t blur_fragment = 2;
constexpr std::size_t present_fragment = 3;
constexpr std::size_t composite_vertex = 4;
constexpr std::size_t composite_fragment = 5;

VkVertexInputAttributeDescription attribute(
    std::uint32_t location, VkFormat format, std::size_t offset) {
  return {.location = location, .binding = 0, .format = format,
          .offset = static_cast<std::uint32_t>(offset)};
}

struct PipelineRequest {
  std::size_t vertex_shader = fullscreen_vertex;
  std::size_t fragment_shader = present_fragment;
  bool composite = false;
};

Result<VkPipeline> create_pipeline(
    VkDevice device,
    const VulkanBackdropBlurResources& resources,
    PipelineRequest request) {
  std::array<VkShaderModule, 2> modules{};
  const std::array<std::size_t, 2> shaders{
      request.vertex_shader, request.fragment_shader};
  Result<void> status{};
  for (std::size_t index = 0; index < modules.size() && status; ++index) {
    const VkShaderModuleCreateInfo info = vulkan_rounded_rect_shader_module_create_info(
        vulkan_backdrop_blur_shader_spirv(shaders[index]));
    status = require_vk_success(
        vkCreateShaderModule(device, &info, nullptr, &modules[index]),
        "vkCreateShaderModule for backdrop blur failed");
  }
  const VulkanRoundedRectShaderModules stage_modules{
      .vertex = modules[0], .fragment = modules[1]};
  const auto stages = vulkan_rounded_rect_shader_stage_create_infos(stage_modules);
  const VkVertexInputBindingDescription binding{
      .binding = 0,
      .stride = sizeof(VulkanBackdropBlurVertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
  constexpr VkFormat vec2 = VK_FORMAT_R32G32_SFLOAT;
  constexpr VkFormat vec4 = VK_FORMAT_R32G32B32A32_SFLOAT;
  const std::array attributes{
      attribute(0, vec2, offsetof(VulkanBackdropBlurVertex, position)),
      attribute(1, vec2, offsetof(VulkanBackdropBlurVertex, local)),
      attribute(2, vec4, offsetof(VulkanBackdropBlurVertex, rect)),
      attribute(3, vec4, offsetof(VulkanBackdropBlurVertex, radii)),
      attribute(4, vec4, offsetof(VulkanBackdropBlurVertex, tint)),
      attribute(5, vec4, offsetof(VulkanBackdropBlurVertex, region)),
  };
  const VkPipelineVertexInputStateCreateInfo vertex_input{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = request.composite ? 1U : 0U,
      .pVertexBindingDescriptions = &binding,
      .vertexAttributeDescriptionCount =
          request.composite ? static_cast<std::uint32_t>(attributes.size()) : 0U,
      .pVertexAttributeDescriptions = attributes.data(),
  };
  const auto input_assembly = vulkan_rounded_rect_input_assembly_state();
  const auto viewport = vulkan_rounded_rect_viewport_state();
  const auto rasterization = vulkan_rounded_rect_rasterization_state();
  const auto multisample = vulkan_rounded_rect_multisample_state();
  const auto depth_stencil = vulkan_rounded_rect_depth_stencil_state();
  VkPipelineColorBlendAttachmentState blend =
      vulkan_rounded_rect_blend_attachment_state();
  blend.blendEnable = request.composite ? VK_TRUE : VK_FALSE;
  const VkPipelineColorBlendStateCreateInfo color_blend{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &blend,
  };
  const auto dynamic_states = vulkan_rounded_rect_dynamic_states();
  const VkPipelineDynamicStateCreateInfo dynamic_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data(),
  };
  // Every offscreen pass and the swapchain pass use one color attachment
  // of the same format, so one render pass serves all of them.
  const VkGraphicsPipelineCreateInfo info{
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
      .layout = resources.pipeline_layout,
      .renderPass = resources.scene_load_pass,
      .subpass = 0,
      .basePipelineIndex = -1,
  };
  VkPipeline pipeline = VK_NULL_HANDLE;
  if (status) {
    status = require_vk_success(
        vkCreateGraphicsPipelines(
            device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline),
        "vkCreateGraphicsPipelines for backdrop blur failed");
  }
  for (VkShaderModule module : modules) {
    if (module != VK_NULL_HANDLE) vkDestroyShaderModule(device, module, nullptr);
  }
  if (!status) return std::unexpected(status.error());
  return pipeline;
}

} // namespace

Result<void> vulkan_create_backdrop_blur_pipelines(
    VkDevice device,
    VulkanBackdropBlurResources& resources) {
  const VkPushConstantRange push_range{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(VulkanBackdropBlurPushConstants),
  };
  const VkPipelineLayoutCreateInfo layout_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &resources.set_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_range,
  };
  if (auto result = require_vk_success(
          vkCreatePipelineLayout(
              device, &layout_info, nullptr, &resources.pipeline_layout),
          "vkCreatePipelineLayout for backdrop blur failed");
      !result) {
    return result;
  }
  const std::array<std::pair<VkPipeline*, PipelineRequest>, 4> requests{{
      {&resources.downsample_pipeline, {.fragment_shader = downsample_fragment}},
      {&resources.blur_pipeline, {.fragment_shader = blur_fragment}},
      {&resources.composite_pipeline,
       {.vertex_shader = composite_vertex,
        .fragment_shader = composite_fragment,
        .composite = true}},
      {&resources.present_pipeline, {.fragment_shader = present_fragment}},
  }};
  for (const auto& [target, request] : requests) {
    auto pipeline = create_pipeline(device, resources, request);
    if (!pipeline) return std::unexpected(pipeline.error());
    *target = *pipeline;
  }
  return {};
}

} // namespace cgpui
