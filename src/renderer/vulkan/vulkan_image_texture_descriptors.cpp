#include "vulkan_image_texture_descriptors_internal.hpp"

#include <array>

namespace cgpui {

VkDescriptorSetLayoutBinding vulkan_image_texture_descriptor_layout_binding() {
  return VkDescriptorSetLayoutBinding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
  };
}

VkDescriptorPoolSize vulkan_image_texture_descriptor_pool_size() {
  return VkDescriptorPoolSize{
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = static_cast<std::uint32_t>(
          vulkan_image_texture_descriptor_capacity *
          vulkan_image_texture_descriptor_sets_per_texture),
  };
}

VkDescriptorImageInfo vulkan_image_texture_descriptor_image_info(
    VkSampler sampler,
    VkImageView image_view) {
  return VkDescriptorImageInfo{
      .sampler = sampler,
      .imageView = image_view,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
}

VkWriteDescriptorSet vulkan_image_texture_descriptor_write(
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

VkDescriptorSet vulkan_image_texture_descriptor_set(
    const VulkanImageTextureResource& resource,
    ImageSamplingMode mode) {
  return mode == ImageSamplingMode::nearest ? resource.nearest_descriptor_set
                                             : resource.linear_descriptor_set;
}

Result<void> vulkan_create_image_texture_descriptor_resources(
    VkDevice device,
    VulkanImageTextureResources& resources) {
  if (device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image texture descriptors require a device"));
  }
  const VkDescriptorSetLayoutBinding binding =
      vulkan_image_texture_descriptor_layout_binding();
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
          "vkCreateDescriptorSetLayout for image texture failed");
      !result) {
    return result;
  }

  const VkDescriptorPoolSize pool_size =
      vulkan_image_texture_descriptor_pool_size();
  const VkDescriptorPoolCreateInfo pool_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
      .maxSets = pool_size.descriptorCount,
      .poolSizeCount = 1,
      .pPoolSizes = &pool_size,
  };
  if (auto result = require_vk_success(
          vkCreateDescriptorPool(
              device,
              &pool_info,
              nullptr,
              &resources.descriptor_pool),
          "vkCreateDescriptorPool for image texture failed");
      !result) {
    vulkan_destroy_image_texture_descriptor_resources(device, resources);
    return result;
  }

  auto nearest = vulkan_create_image_texture_sampler(
      device, ImageSamplingMode::nearest);
  if (!nearest) {
    vulkan_destroy_image_texture_descriptor_resources(device, resources);
    return std::unexpected(nearest.error());
  }
  resources.nearest_sampler = *nearest;
  auto linear =
      vulkan_create_image_texture_sampler(device, ImageSamplingMode::linear);
  if (!linear) {
    vulkan_destroy_image_texture_descriptor_resources(device, resources);
    return std::unexpected(linear.error());
  }
  resources.linear_sampler = *linear;
  return {};
}

Result<void> vulkan_bind_image_texture_resource_descriptors(
    VkDevice device,
    const VulkanImageTextureResources& resources,
    VulkanImageTextureResource& resource) {
  if (device == VK_NULL_HANDLE ||
      resources.descriptor_pool == VK_NULL_HANDLE ||
      resources.descriptor_set_layout == VK_NULL_HANDLE ||
      resources.nearest_sampler == VK_NULL_HANDLE ||
      resources.linear_sampler == VK_NULL_HANDLE ||
      resource.image_view == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image texture descriptor binding requires complete resources"));
  }
  const std::array<VkDescriptorSetLayout, 2> layouts{
      resources.descriptor_set_layout,
      resources.descriptor_set_layout,
  };
  std::array<VkDescriptorSet, 2> descriptor_sets{};
  const VkDescriptorSetAllocateInfo allocation_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = resources.descriptor_pool,
      .descriptorSetCount = static_cast<std::uint32_t>(descriptor_sets.size()),
      .pSetLayouts = layouts.data(),
  };
  if (auto result = require_vk_success(
          vkAllocateDescriptorSets(
              device, &allocation_info, descriptor_sets.data()),
          "vkAllocateDescriptorSets for image texture failed");
      !result) {
    return result;
  }

  const std::array<VkDescriptorImageInfo, 2> image_infos{
      vulkan_image_texture_descriptor_image_info(
          resources.nearest_sampler, resource.image_view),
      vulkan_image_texture_descriptor_image_info(
          resources.linear_sampler, resource.image_view),
  };
  const std::array<VkWriteDescriptorSet, 2> writes{
      vulkan_image_texture_descriptor_write(descriptor_sets[0], image_infos[0]),
      vulkan_image_texture_descriptor_write(descriptor_sets[1], image_infos[1]),
  };
  vkUpdateDescriptorSets(
      device,
      static_cast<std::uint32_t>(writes.size()),
      writes.data(),
      0,
      nullptr);
  resource.descriptor_pool = resources.descriptor_pool;
  resource.nearest_descriptor_set = descriptor_sets[0];
  resource.linear_descriptor_set = descriptor_sets[1];
  return {};
}

void vulkan_destroy_image_texture_descriptor_resources(
    VkDevice device,
    VulkanImageTextureResources& resources) {
  if (resources.linear_sampler != VK_NULL_HANDLE) {
    vkDestroySampler(device, resources.linear_sampler, nullptr);
    resources.linear_sampler = VK_NULL_HANDLE;
  }
  if (resources.nearest_sampler != VK_NULL_HANDLE) {
    vkDestroySampler(device, resources.nearest_sampler, nullptr);
    resources.nearest_sampler = VK_NULL_HANDLE;
  }
  if (resources.descriptor_pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device, resources.descriptor_pool, nullptr);
    resources.descriptor_pool = VK_NULL_HANDLE;
  }
  if (resources.descriptor_set_layout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(
        device, resources.descriptor_set_layout, nullptr);
    resources.descriptor_set_layout = VK_NULL_HANDLE;
  }
}

} // namespace cgpui
