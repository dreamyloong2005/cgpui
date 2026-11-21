#include "vulkan_image_pipeline_internal.hpp"

namespace cgpui {
namespace {

// SPIR-V compiled from Slang by scripts/dev/compile_shaders.py.
#include "generated/image_spirv.inc"

} // namespace

std::span<const std::uint32_t> vulkan_image_vertex_shader_spirv() {
  return image_vertex_shader_spirv;
}

std::span<const std::uint32_t> vulkan_image_fragment_shader_spirv() {
  return image_fragment_shader_spirv;
}

VkShaderModuleCreateInfo vulkan_image_shader_module_create_info(
    std::span<const std::uint32_t> spirv) {
  return VkShaderModuleCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = spirv.size_bytes(),
      .pCode = spirv.data(),
  };
}

} // namespace cgpui
