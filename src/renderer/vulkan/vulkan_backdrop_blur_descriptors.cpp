#include "vulkan_backdrop_blur_resources_internal.hpp"

namespace cgpui {

// One combined image sampler per target; texelFetch ignores the filter and
// the composite upsamples the blurred region with linear filtering.
Result<void> vulkan_create_backdrop_blur_descriptors(
    VkDevice device,
    VulkanBackdropBlurResources& resources) {
  const VkSamplerCreateInfo sampler_info{
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
      .maxLod = 0.0F,
  };
  if (auto result = require_vk_success(
          vkCreateSampler(device, &sampler_info, nullptr, &resources.sampler),
          "vkCreateSampler for backdrop blur failed");
      !result) {
    return result;
  }
  const VkDescriptorSetLayoutBinding binding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
  };
  const VkDescriptorSetLayoutCreateInfo layout_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &binding,
  };
  if (auto result = require_vk_success(
          vkCreateDescriptorSetLayout(
              device, &layout_info, nullptr, &resources.set_layout),
          "vkCreateDescriptorSetLayout for backdrop blur failed");
      !result) {
    return result;
  }
  const auto count = static_cast<std::uint32_t>(resources.targets.size());
  const VkDescriptorPoolSize pool_size{
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = count,
  };
  const VkDescriptorPoolCreateInfo pool_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = count,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size,
  };
  if (auto result = require_vk_success(
          vkCreateDescriptorPool(
              device, &pool_info, nullptr, &resources.descriptor_pool),
          "vkCreateDescriptorPool for backdrop blur failed");
      !result) {
    return result;
  }
  for (VulkanBackdropBlurTarget& target : resources.targets) {
    const VkDescriptorSetAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = resources.descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &resources.set_layout,
    };
    if (auto result = require_vk_success(
            vkAllocateDescriptorSets(
                device, &allocate_info, &target.descriptor_set),
            "vkAllocateDescriptorSets for backdrop blur failed");
        !result) {
      return result;
    }
    const VkDescriptorImageInfo image_info{
        .sampler = resources.sampler,
        .imageView = target.view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    const VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = target.descriptor_set,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &image_info,
    };
    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
  }
  return {};
}

} // namespace cgpui
