#include "vulkan_image_texture_resources_internal.hpp"

#include <algorithm>

namespace cgpui {

namespace {

bool same_allocation_identity(
    const ImageAssetDescriptor& lhs,
    const ImageAssetDescriptor& rhs) {
  return lhs.id == rhs.id && lhs.pixel_width == rhs.pixel_width &&
         lhs.pixel_height == rhs.pixel_height && lhs.format == rhs.format;
}

} // namespace

bool vulkan_image_texture_descriptor_valid(
    const ImageAssetDescriptor& descriptor) {
  return descriptor.id.value != 0 && descriptor.pixel_width != 0 &&
         descriptor.pixel_height != 0 &&
         vulkan_image_texture_format(descriptor.format) != VK_FORMAT_UNDEFINED;
}

bool vulkan_image_texture_resource_matches(
    const VulkanImageTextureResource& resource,
    const ImageAssetDescriptor& descriptor) {
  return resource.descriptor.id == descriptor.id &&
         resource.extent.width == descriptor.pixel_width &&
         resource.extent.height == descriptor.pixel_height &&
         resource.format == vulkan_image_texture_format(descriptor.format);
}

Result<void> vulkan_validate_image_texture_resource_requests(
    std::span<const ImageDraw> image_draws) {
  for (std::size_t index = 0; index < image_draws.size(); ++index) {
    const ImageAssetDescriptor& descriptor = image_draws[index].asset;
    if (!vulkan_image_texture_descriptor_valid(descriptor)) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "image texture request contains an invalid descriptor"));
    }
    for (std::size_t prior = 0; prior < index; ++prior) {
      const ImageAssetDescriptor& existing = image_draws[prior].asset;
      if (existing.id == descriptor.id &&
          !same_allocation_identity(existing, descriptor)) {
        return std::unexpected(vulkan_error(
            ErrorCode::renderer_initialization_failed,
            "image texture requests conflict for one asset id"));
      }
    }
  }
  return {};
}

VulkanImageTextureResource* vulkan_find_image_texture_resource(
    VulkanImageTextureResources& resources,
    ImageAssetId asset_id) {
  const auto texture = std::ranges::find_if(
      resources.textures,
      [asset_id](const VulkanImageTextureResource& candidate) {
        return candidate.descriptor.id == asset_id;
      });
  return texture == resources.textures.end() ? nullptr : &*texture;
}

const VulkanImageTextureResource* vulkan_find_image_texture_resource(
    const VulkanImageTextureResources& resources,
    ImageAssetId asset_id) {
  const auto texture = std::ranges::find_if(
      resources.textures,
      [asset_id](const VulkanImageTextureResource& candidate) {
        return candidate.descriptor.id == asset_id;
      });
  return texture == resources.textures.end() ? nullptr : &*texture;
}

void vulkan_destroy_image_texture_resources(
    VkDevice device,
    VulkanImageTextureResources& resources) {
  for (VulkanImageTextureResource& texture : resources.textures) {
    vulkan_destroy_image_texture_resource(device, texture);
  }
  resources.textures.clear();
}

Result<void> vulkan_update_image_texture_resources(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const ImageDraw> image_draws,
    VulkanImageTextureResources& resources) {
  if (auto result =
          vulkan_validate_image_texture_resource_requests(image_draws);
      !result) {
    return result;
  }
  if (!image_draws.empty() &&
      (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE)) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image texture update requires Vulkan devices"));
  }

  for (const ImageDraw& draw : image_draws) {
    VulkanImageTextureResource* texture =
        vulkan_find_image_texture_resource(resources, draw.asset.id);
    if (texture != nullptr &&
        vulkan_image_texture_resource_matches(*texture, draw.asset)) {
      texture->descriptor = draw.asset;
      continue;
    }
    if (texture != nullptr) {
      const auto found = std::ranges::find_if(
          resources.textures,
          [texture](const VulkanImageTextureResource& candidate) {
            return &candidate == texture;
          });
      vulkan_destroy_image_texture_resource(device, *texture);
      resources.textures.erase(found);
    }

    auto created = vulkan_create_image_texture_resource(
        physical_device,
        device,
        draw.asset);
    if (!created) {
      return std::unexpected(created.error());
    }
    resources.textures.push_back(*created);
  }
  return {};
}

} // namespace cgpui
