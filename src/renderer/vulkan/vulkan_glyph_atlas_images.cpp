#include "vulkan_glyph_atlas_resources_internal.hpp"

#include "vulkan_device_internal.hpp"

#include <cmath>
#include <limits>

namespace cgpui {
namespace {

Result<VkExtent2D> glyph_atlas_extent(Size size) {
  constexpr float max_dimension =
      static_cast<float>(std::numeric_limits<std::uint32_t>::max());
  if (!std::isfinite(size.width) || !std::isfinite(size.height) ||
      size.width <= 0.0F || size.height <= 0.0F ||
      size.width > max_dimension || size.height > max_dimension) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "invalid Vulkan glyph atlas image extent"));
  }
  return VkExtent2D{
      .width = static_cast<std::uint32_t>(std::ceil(size.width)),
      .height = static_cast<std::uint32_t>(std::ceil(size.height)),
  };
}

} // namespace

Result<VkSampler> vulkan_create_glyph_atlas_sampler(VkDevice device) {
  VkSampler sampler = VK_NULL_HANDLE;
  const VkSamplerCreateInfo create_info{
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
          vkCreateSampler(device, &create_info, nullptr, &sampler),
          "vkCreateSampler for glyph atlas failed");
      !result) {
    return std::unexpected(result.error());
  }
  return sampler;
}

Result<VulkanGlyphAtlasPageResource> vulkan_create_glyph_atlas_page_resource(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkDescriptorSetLayout descriptor_set_layout,
    VkDescriptorPool descriptor_pool,
    VkSampler sampler,
    const GlyphAtlasProductionResourceRecord& record) {
  auto extent = glyph_atlas_extent(record.image.size);
  if (!extent) {
    return std::unexpected(extent.error());
  }

  VulkanGlyphAtlasPageResource resource{
      .page_index = record.page_index,
      .generation = record.generation,
      .extent = *extent,
  };
  const VkImageCreateInfo image_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8_UNORM,
      .extent = VkExtent3D{.width = extent->width, .height = extent->height, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };
  if (auto result = require_vk_success(
          vkCreateImage(device, &image_info, nullptr, &resource.image),
          "vkCreateImage for glyph atlas failed");
      !result) {
    return std::unexpected(result.error());
  }

  VkMemoryRequirements requirements{};
  vkGetImageMemoryRequirements(device, resource.image, &requirements);
  const auto memory_type = vulkan_find_memory_type(
      physical_device,
      requirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (!memory_type) {
    vulkan_destroy_glyph_atlas_page_resource(device, descriptor_pool, resource);
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        memory_type.error().message));
  }

  const VkMemoryAllocateInfo allocation_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = requirements.size,
      .memoryTypeIndex = *memory_type,
  };
  if (auto result = require_vk_success(
          vkAllocateMemory(device, &allocation_info, nullptr, &resource.memory),
          "vkAllocateMemory for glyph atlas failed");
      !result) {
    vulkan_destroy_glyph_atlas_page_resource(device, descriptor_pool, resource);
    return std::unexpected(result.error());
  }
  if (auto result = require_vk_success(
          vkBindImageMemory(device, resource.image, resource.memory, 0),
          "vkBindImageMemory for glyph atlas failed");
      !result) {
    vulkan_destroy_glyph_atlas_page_resource(device, descriptor_pool, resource);
    return std::unexpected(result.error());
  }

  const VkImageViewCreateInfo view_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = resource.image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8_UNORM,
      .subresourceRange =
          VkImageSubresourceRange{
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
  if (auto result = require_vk_success(
          vkCreateImageView(device, &view_info, nullptr, &resource.image_view),
          "vkCreateImageView for glyph atlas failed");
      !result) {
    vulkan_destroy_glyph_atlas_page_resource(device, descriptor_pool, resource);
    return std::unexpected(result.error());
  }

  auto descriptor_set = vulkan_allocate_glyph_atlas_descriptor_set(
      device,
      descriptor_pool,
      descriptor_set_layout,
      sampler,
      resource.image_view);
  if (!descriptor_set) {
    vulkan_destroy_glyph_atlas_page_resource(device, descriptor_pool, resource);
    return std::unexpected(descriptor_set.error());
  }
  resource.descriptor_set = *descriptor_set;
  return resource;
}

} // namespace cgpui
