#pragma once

#include "vulkan_platform_internal.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace cgpui {

struct VulkanImageTextureResource {
  ImageAssetDescriptor descriptor;
  VkFormat format = VK_FORMAT_UNDEFINED;
  VkExtent2D extent{};
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkImageView image_view = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
  VkDescriptorSet nearest_descriptor_set = VK_NULL_HANDLE;
  VkDescriptorSet linear_descriptor_set = VK_NULL_HANDLE;
  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  std::uint64_t last_used_frame = 0;
};

struct VulkanImageTextureResources {
  VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
  VkSampler nearest_sampler = VK_NULL_HANDLE;
  VkSampler linear_sampler = VK_NULL_HANDLE;
  std::uint64_t frame_index = 0;
  std::vector<VulkanImageTextureResource> textures;
};

[[nodiscard]] VkFormat vulkan_image_texture_format(ImageFormat format);
[[nodiscard]] bool vulkan_image_texture_descriptor_valid(
    const ImageAssetDescriptor& descriptor);
[[nodiscard]] VkImageCreateInfo vulkan_image_texture_create_info(
    const ImageAssetDescriptor& descriptor);
[[nodiscard]] VkImageViewCreateInfo vulkan_image_texture_view_create_info(
    VkImage image,
    VkFormat format);
[[nodiscard]] bool vulkan_image_texture_resource_matches(
    const VulkanImageTextureResource& resource,
    const ImageAssetDescriptor& descriptor);
[[nodiscard]] Result<void> vulkan_validate_image_texture_resource_requests(
    std::span<const ImageDraw> image_draws);
[[nodiscard]] VulkanImageTextureResource* vulkan_find_image_texture_resource(
    VulkanImageTextureResources& resources,
    ImageAssetId asset_id);
[[nodiscard]] const VulkanImageTextureResource*
vulkan_find_image_texture_resource(
    const VulkanImageTextureResources& resources,
    ImageAssetId asset_id);

Result<VulkanImageTextureResource> vulkan_create_image_texture_resource(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const ImageAssetDescriptor& descriptor);
void vulkan_destroy_image_texture_resource(
    VkDevice device,
    VulkanImageTextureResource& resource);
void vulkan_destroy_image_texture_resources(
    VkDevice device,
    VulkanImageTextureResources& resources);
Result<void> vulkan_ensure_image_texture_resource(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const ImageAssetDescriptor& descriptor,
    VulkanImageTextureResources& resources);
Result<void> vulkan_update_image_texture_resources(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const ImageDraw> image_draws,
    VulkanImageTextureResources& resources);

} // namespace cgpui
