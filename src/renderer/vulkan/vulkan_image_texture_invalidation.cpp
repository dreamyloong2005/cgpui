#include "vulkan_image_texture_invalidation_internal.hpp"

#include <algorithm>

namespace cgpui {

std::size_t vulkan_invalidate_image_texture_resources(
    VkDevice device,
    std::span<const ImageAssetId> invalidations,
    VulkanImageTextureResources& resources) {
  std::size_t invalidated = 0;
  for (ImageAssetId asset_id : invalidations) {
    if (asset_id.value == 0) {
      continue;
    }
    const auto texture = std::ranges::find_if(
        resources.textures,
        [asset_id](const VulkanImageTextureResource& candidate) {
          return candidate.descriptor.id == asset_id;
        });
    if (texture == resources.textures.end()) {
      continue;
    }
    vulkan_destroy_image_texture_resource(device, *texture);
    resources.textures.erase(texture);
    ++invalidated;
  }
  return invalidated;
}

} // namespace cgpui
