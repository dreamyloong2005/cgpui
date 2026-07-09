#include "vulkan_glyph_atlas_resources_internal.hpp"

namespace cgpui {

VkDescriptorSetLayoutBinding vulkan_glyph_atlas_descriptor_layout_binding() {
  return VkDescriptorSetLayoutBinding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
  };
}

VkDescriptorPoolSize vulkan_glyph_atlas_descriptor_pool_size(
    std::uint32_t descriptor_count) {
  return VkDescriptorPoolSize{
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = descriptor_count,
  };
}

VkDescriptorImageInfo vulkan_glyph_atlas_descriptor_image_info(
    VkSampler sampler,
    VkImageView image_view) {
  return VkDescriptorImageInfo{
      .sampler = sampler,
      .imageView = image_view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
}

VkWriteDescriptorSet vulkan_glyph_atlas_descriptor_write(
    VkDescriptorSet descriptor_set,
    const VkDescriptorImageInfo& image_info) {
  return VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptor_set,
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &image_info,
  };
}

Result<void> vulkan_create_glyph_atlas_descriptor_resources(
    VkDevice device,
    VulkanGlyphAtlasResources& resources) {
  const VkDescriptorSetLayoutBinding binding =
      vulkan_glyph_atlas_descriptor_layout_binding();
  const VkDescriptorSetLayoutCreateInfo layout_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &binding,
  };
  if (auto result = require_vk_success(
          vkCreateDescriptorSetLayout(
              device,
              &layout_info,
              nullptr,
              &resources.descriptor_set_layout),
          "vkCreateDescriptorSetLayout for glyph atlas failed");
      !result) {
    return result;
  }

  const VkDescriptorPoolSize pool_size =
      vulkan_glyph_atlas_descriptor_pool_size(
          static_cast<std::uint32_t>(
              vulkan_glyph_atlas_descriptor_capacity));
  const VkDescriptorPoolCreateInfo pool_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      .maxSets = static_cast<std::uint32_t>(
          vulkan_glyph_atlas_descriptor_capacity),
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size,
  };
  if (auto result = require_vk_success(
          vkCreateDescriptorPool(
              device,
              &pool_info,
              nullptr,
              &resources.descriptor_pool),
          "vkCreateDescriptorPool for glyph atlas failed");
      !result) {
    vulkan_destroy_glyph_atlas_resources(device, resources);
    return result;
  }

  auto sampler = vulkan_create_glyph_atlas_sampler(device);
  if (!sampler) {
    vulkan_destroy_glyph_atlas_resources(device, resources);
    return std::unexpected(sampler.error());
  }
  resources.sampler = *sampler;
  return {};
}

Result<VkDescriptorSet> vulkan_allocate_glyph_atlas_descriptor_set(
    VkDevice device,
    VkDescriptorPool descriptor_pool,
    VkDescriptorSetLayout descriptor_set_layout,
    VkSampler sampler,
    VkImageView image_view) {
  VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
  const VkDescriptorSetAllocateInfo allocation_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptor_set_layout,
  };
  if (auto result = require_vk_success(
          vkAllocateDescriptorSets(device, &allocation_info, &descriptor_set),
          "vkAllocateDescriptorSets for glyph atlas failed");
      !result) {
    return std::unexpected(result.error());
  }

  const VkDescriptorImageInfo image_info =
      vulkan_glyph_atlas_descriptor_image_info(sampler, image_view);
  const VkWriteDescriptorSet write =
      vulkan_glyph_atlas_descriptor_write(descriptor_set, image_info);
  vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
  return descriptor_set;
}

} // namespace cgpui
