#include "vulkan_image_pipeline_internal.hpp"

namespace cgpui {
namespace {

Result<VkShaderModule> create_shader_module(
    VkDevice device,
    std::span<const std::uint32_t> spirv,
    const char* failure_message) {
  VkShaderModule module = VK_NULL_HANDLE;
  const VkShaderModuleCreateInfo create_info =
      vulkan_image_shader_module_create_info(spirv);
  if (auto result = require_vk_success(
          vkCreateShaderModule(device, &create_info, nullptr, &module),
          failure_message);
      !result) {
    return std::unexpected(result.error());
  }
  return module;
}

} // namespace

Result<void> vulkan_create_image_shader_modules(
    VkDevice device,
    VulkanImageShaderModules& modules) {
  if (device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image shader modules require a Vulkan device"));
  }
  vulkan_destroy_image_shader_modules(device, modules);

  auto vertex = create_shader_module(
      device,
      vulkan_image_vertex_shader_spirv(),
      "vkCreateShaderModule for image vertex shader failed");
  if (!vertex) {
    return std::unexpected(vertex.error());
  }
  modules.vertex = *vertex;

  auto fragment = create_shader_module(
      device,
      vulkan_image_fragment_shader_spirv(),
      "vkCreateShaderModule for image fragment shader failed");
  if (!fragment) {
    vulkan_destroy_image_shader_modules(device, modules);
    return std::unexpected(fragment.error());
  }
  modules.fragment = *fragment;
  return {};
}

void vulkan_destroy_image_shader_modules(
    VkDevice device,
    VulkanImageShaderModules& modules) {
  if (device == VK_NULL_HANDLE) {
    return;
  }
  if (modules.fragment != VK_NULL_HANDLE) {
    vkDestroyShaderModule(device, modules.fragment, nullptr);
  }
  if (modules.vertex != VK_NULL_HANDLE) {
    vkDestroyShaderModule(device, modules.vertex, nullptr);
  }
  modules = {};
}

std::array<VkPipelineShaderStageCreateInfo, 2>
vulkan_image_shader_stage_create_infos(
    const VulkanImageShaderModules& modules) {
  return {
      VkPipelineShaderStageCreateInfo{
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = VK_SHADER_STAGE_VERTEX_BIT,
          .module = modules.vertex,
          .pName = "main",
      },
      VkPipelineShaderStageCreateInfo{
          .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
          .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
          .module = modules.fragment,
          .pName = "main",
      },
  };
}

} // namespace cgpui
