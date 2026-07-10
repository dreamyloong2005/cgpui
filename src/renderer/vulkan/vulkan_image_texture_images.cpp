#include "vulkan_image_texture_resources_internal.hpp"

#include "vulkan_device_internal.hpp"

#include <array>

namespace cgpui {

VkFormat vulkan_image_texture_format(ImageFormat format) {
  return format == ImageFormat::rgba8_unorm ? VK_FORMAT_R8G8B8A8_UNORM
                                            : VK_FORMAT_UNDEFINED;
}

VkImageCreateInfo vulkan_image_texture_create_info(
    const ImageAssetDescriptor& descriptor) {
  return VkImageCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = vulkan_image_texture_format(descriptor.format),
      .extent =
          VkExtent3D{
              .width = descriptor.pixel_width,
              .height = descriptor.pixel_height,
              .depth = 1,
          },
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
}

VkImageViewCreateInfo vulkan_image_texture_view_create_info(
    VkImage image,
    VkFormat format) {
  return VkImageViewCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .subresourceRange =
          VkImageSubresourceRange{
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
}

Result<VulkanImageTextureResource> vulkan_create_image_texture_resource(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const ImageAssetDescriptor& descriptor) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE ||
      !vulkan_image_texture_descriptor_valid(descriptor)) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image texture creation requires a valid device and descriptor"));
  }

  VulkanImageTextureResource resource{
      .descriptor = descriptor,
      .format = vulkan_image_texture_format(descriptor.format),
      .extent =
          VkExtent2D{
              .width = descriptor.pixel_width,
              .height = descriptor.pixel_height,
          },
  };
  const VkImageCreateInfo image_info =
      vulkan_image_texture_create_info(descriptor);
  if (auto result = require_vk_success(
          vkCreateImage(device, &image_info, nullptr, &resource.image),
          "vkCreateImage for image texture failed");
      !result) {
    return std::unexpected(result.error());
  }

  VkMemoryRequirements requirements{};
  vkGetImageMemoryRequirements(device, resource.image, &requirements);
  auto memory_type = vulkan_find_memory_type(
      physical_device,
      requirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (!memory_type) {
    vulkan_destroy_image_texture_resource(device, resource);
    return std::unexpected(memory_type.error());
  }
  const VkMemoryAllocateInfo allocation_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = requirements.size,
      .memoryTypeIndex = *memory_type,
  };
  if (auto result = require_vk_success(
          vkAllocateMemory(device, &allocation_info, nullptr, &resource.memory),
          "vkAllocateMemory for image texture failed");
      !result) {
    vulkan_destroy_image_texture_resource(device, resource);
    return std::unexpected(result.error());
  }
  if (auto result = require_vk_success(
          vkBindImageMemory(device, resource.image, resource.memory, 0),
          "vkBindImageMemory for image texture failed");
      !result) {
    vulkan_destroy_image_texture_resource(device, resource);
    return std::unexpected(result.error());
  }

  const VkImageViewCreateInfo view_info =
      vulkan_image_texture_view_create_info(resource.image, resource.format);
  if (auto result = require_vk_success(
          vkCreateImageView(
              device,
              &view_info,
              nullptr,
              &resource.image_view),
          "vkCreateImageView for image texture failed");
      !result) {
    vulkan_destroy_image_texture_resource(device, resource);
    return std::unexpected(result.error());
  }
  return resource;
}

void vulkan_destroy_image_texture_resource(
    VkDevice device,
    VulkanImageTextureResource& resource) {
  if (device != VK_NULL_HANDLE) {
    std::array<VkDescriptorSet, 2> descriptor_sets{};
    std::uint32_t descriptor_count = 0;
    if (resource.nearest_descriptor_set != VK_NULL_HANDLE) {
      descriptor_sets[descriptor_count++] = resource.nearest_descriptor_set;
    }
    if (resource.linear_descriptor_set != VK_NULL_HANDLE) {
      descriptor_sets[descriptor_count++] = resource.linear_descriptor_set;
    }
    if (descriptor_count != 0 && resource.descriptor_pool != VK_NULL_HANDLE) {
      (void)vkFreeDescriptorSets(
          device,
          resource.descriptor_pool,
          descriptor_count,
          descriptor_sets.data());
    }
    if (resource.image_view != VK_NULL_HANDLE) {
      vkDestroyImageView(device, resource.image_view, nullptr);
    }
    if (resource.image != VK_NULL_HANDLE) {
      vkDestroyImage(device, resource.image, nullptr);
    }
    if (resource.memory != VK_NULL_HANDLE) {
      vkFreeMemory(device, resource.memory, nullptr);
    }
  }
  resource = {};
}

} // namespace cgpui
