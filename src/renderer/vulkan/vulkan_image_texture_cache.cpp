#include "vulkan_image_texture_cache_internal.hpp"

#include <limits>

namespace cgpui {

void vulkan_begin_image_texture_cache_frame(
    VulkanImageTextureResources& resources) {
  if (resources.frame_index == std::numeric_limits<std::uint64_t>::max()) {
    resources.frame_index = 1;
    for (VulkanImageTextureResource& texture : resources.textures) {
      texture.last_used_frame = resources.frame_index;
    }
    return;
  }
  ++resources.frame_index;
}

void vulkan_touch_image_texture_resource(
    VulkanImageTextureResources& resources,
    ImageAssetId asset_id) {
  if (VulkanImageTextureResource* texture =
          vulkan_find_image_texture_resource(resources, asset_id);
      texture != nullptr) {
    texture->last_used_frame = resources.frame_index;
  }
}

bool vulkan_image_texture_resource_idle(
    const VulkanImageTextureResource& resource,
    std::uint64_t frame_index,
    std::uint64_t max_idle_frames) {
  return frame_index > resource.last_used_frame &&
         frame_index - resource.last_used_frame > max_idle_frames;
}

std::size_t vulkan_evict_idle_image_texture_resources(
    VkDevice device,
    VulkanImageTextureResources& resources,
    std::uint64_t max_idle_frames) {
  std::size_t evicted = 0;
  std::size_t index = 0;
  while (index < resources.textures.size()) {
    VulkanImageTextureResource& texture = resources.textures[index];
    if (!vulkan_image_texture_resource_idle(
            texture, resources.frame_index, max_idle_frames)) {
      ++index;
      continue;
    }
    vulkan_destroy_image_texture_resource(device, texture);
    resources.textures.erase(resources.textures.begin() + index);
    ++evicted;
  }
  return evicted;
}

std::size_t vulkan_prepare_image_texture_cache_frame(
    VkDevice device,
    std::span<const ImageDraw> image_draws,
    std::span<const ImageUploadBatch> image_uploads,
    VulkanImageTextureResources& resources) {
  vulkan_begin_image_texture_cache_frame(resources);
  for (const ImageDraw& draw : image_draws) {
    vulkan_touch_image_texture_resource(resources, draw.asset.id);
  }
  for (const ImageUploadBatch& upload : image_uploads) {
    vulkan_touch_image_texture_resource(resources, upload.image.id);
  }
  return vulkan_evict_idle_image_texture_resources(device, resources);
}

} // namespace cgpui
