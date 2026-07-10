#include "vulkan_image_texture_descriptors_internal.hpp"

namespace cgpui {

VkSamplerCreateInfo vulkan_image_texture_sampler_create_info(
    ImageSamplingMode mode) {
  const bool linear = mode == ImageSamplingMode::linear;
  return VkSamplerCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST,
      .minFilter = linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST,
      .mipmapMode = linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR
                           : VK_SAMPLER_MIPMAP_MODE_NEAREST,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .minLod = 0.0F,
      .maxLod = 0.0F,
  };
}

Result<VkSampler> vulkan_create_image_texture_sampler(
    VkDevice device,
    ImageSamplingMode mode) {
  if (device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image texture sampler creation requires a device"));
  }
  VkSampler sampler = VK_NULL_HANDLE;
  const VkSamplerCreateInfo create_info =
      vulkan_image_texture_sampler_create_info(mode);
  if (auto result = require_vk_success(
          vkCreateSampler(device, &create_info, nullptr, &sampler),
          "vkCreateSampler for image texture failed");
      !result) {
    return std::unexpected(result.error());
  }
  return sampler;
}

} // namespace cgpui
