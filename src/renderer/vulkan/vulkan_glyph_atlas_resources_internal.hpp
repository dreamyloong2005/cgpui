#pragma once

#include "cgpui/renderer/glyph_atlas_production.hpp"
#include "vulkan_platform_internal.hpp"

namespace cgpui {

struct VulkanGlyphAtlasPageResource {
  std::size_t page_index = 0;
  std::size_t generation = 0;
  VkExtent2D extent{};
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkImageView image_view = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
};

struct VulkanGlyphAtlasResources {
  VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
  VkSampler sampler = VK_NULL_HANDLE;
  std::vector<VulkanGlyphAtlasPageResource> pages;
};

[[nodiscard]] VkDescriptorSetLayoutBinding
vulkan_glyph_atlas_descriptor_layout_binding();
[[nodiscard]] VkDescriptorPoolSize vulkan_glyph_atlas_descriptor_pool_size(
    std::uint32_t descriptor_count);
[[nodiscard]] VkDescriptorImageInfo vulkan_glyph_atlas_descriptor_image_info(
    VkSampler sampler,
    VkImageView image_view);
[[nodiscard]] VkWriteDescriptorSet vulkan_glyph_atlas_descriptor_write(
    VkDescriptorSet descriptor_set,
    const VkDescriptorImageInfo& image_info);

Result<void> vulkan_create_glyph_atlas_descriptor_resources(
    VkDevice device,
    VulkanGlyphAtlasResources& resources);
Result<VkDescriptorSet> vulkan_allocate_glyph_atlas_descriptor_set(
    VkDevice device,
    VkDescriptorPool descriptor_pool,
    VkDescriptorSetLayout descriptor_set_layout,
    VkSampler sampler,
    VkImageView image_view);
Result<VkSampler> vulkan_create_glyph_atlas_sampler(VkDevice device);
Result<VulkanGlyphAtlasPageResource> vulkan_create_glyph_atlas_page_resource(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkDescriptorSetLayout descriptor_set_layout,
    VkDescriptorPool descriptor_pool,
    VkSampler sampler,
    const GlyphAtlasProductionResourceRecord& record);
void vulkan_destroy_glyph_atlas_page_resource(
    VkDevice device,
    VkDescriptorPool descriptor_pool,
    VulkanGlyphAtlasPageResource& resource);
void vulkan_destroy_glyph_atlas_resources(
    VkDevice device,
    VulkanGlyphAtlasResources& resources);
Result<void> vulkan_update_glyph_atlas_resources(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const GlyphAtlasProductionPlan& plan,
    VulkanGlyphAtlasResources& resources);

} // namespace cgpui
