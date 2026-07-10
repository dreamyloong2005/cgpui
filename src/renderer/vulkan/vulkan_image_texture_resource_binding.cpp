#include "vulkan_image_texture_descriptors_internal.hpp"

#include <algorithm>

namespace cgpui {

Result<void> vulkan_ensure_image_texture_resource(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const ImageAssetDescriptor& descriptor,
    VulkanImageTextureResources& resources) {
  if (!vulkan_image_texture_descriptor_valid(descriptor) ||
      physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image texture resource requires a valid descriptor and device"));
  }
  VulkanImageTextureResource* texture =
      vulkan_find_image_texture_resource(resources, descriptor.id);
  if (texture != nullptr &&
      vulkan_image_texture_resource_matches(*texture, descriptor)) {
    texture->descriptor = descriptor;
    texture->last_used_frame = resources.frame_index;
    return {};
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
  if (resources.textures.size() >= vulkan_image_texture_descriptor_capacity) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image texture descriptor capacity exceeded"));
  }

  auto created = vulkan_create_image_texture_resource(
      physical_device,
      device,
      descriptor);
  if (!created) {
    return std::unexpected(created.error());
  }
  if (auto result = vulkan_bind_image_texture_resource_descriptors(
          device, resources, *created);
      !result) {
    vulkan_destroy_image_texture_resource(device, *created);
    return result;
  }
  created->last_used_frame = resources.frame_index;
  resources.textures.push_back(*created);
  return {};
}

} // namespace cgpui
