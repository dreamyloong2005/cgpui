#pragma once

#include "cgpui/renderer/image_sampling.hpp"
#include "vulkan_image_texture_resources_internal.hpp"

namespace cgpui {

inline constexpr std::size_t vulkan_image_texture_descriptor_capacity = 256;
inline constexpr std::size_t vulkan_image_texture_descriptor_sets_per_texture =
    2;

[[nodiscard]] VkSamplerCreateInfo vulkan_image_texture_sampler_create_info(
    ImageSamplingMode mode);
[[nodiscard]] VkDescriptorSetLayoutBinding
vulkan_image_texture_descriptor_layout_binding();
[[nodiscard]] VkDescriptorPoolSize
vulkan_image_texture_descriptor_pool_size();
[[nodiscard]] VkDescriptorImageInfo vulkan_image_texture_descriptor_image_info(
    VkSampler sampler,
    VkImageView image_view);
[[nodiscard]] VkWriteDescriptorSet vulkan_image_texture_descriptor_write(
    VkDescriptorSet descriptor_set,
    const VkDescriptorImageInfo& image_info);
[[nodiscard]] VkDescriptorSet vulkan_image_texture_descriptor_set(
    const VulkanImageTextureResource& resource,
    ImageSamplingMode mode);

Result<VkSampler> vulkan_create_image_texture_sampler(
    VkDevice device,
    ImageSamplingMode mode);
Result<void> vulkan_create_image_texture_descriptor_resources(
    VkDevice device,
    VulkanImageTextureResources& resources);
Result<void> vulkan_bind_image_texture_resource_descriptors(
    VkDevice device,
    const VulkanImageTextureResources& resources,
    VulkanImageTextureResource& resource);
void vulkan_destroy_image_texture_descriptor_resources(
    VkDevice device,
    VulkanImageTextureResources& resources);

} // namespace cgpui
