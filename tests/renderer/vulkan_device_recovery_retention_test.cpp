#include "vulkan_image_retention_internal.hpp"
#include "vulkan_image_texture_cache_internal.hpp"
#include <array>
#include <limits>

using namespace cgpui;
namespace {
ImageUploadBatch batch(std::uint64_t id, std::uint8_t value = 1) {
  return {.image = {.id = {id}, .pixel_width = 1, .pixel_height = 1, .stride = 4, .byte_size = 4},
      .uploads = {{{id}, 1, 1, 4, 0, 4}}, .rgba = {value, 0, 0, 255}};
}
VulkanImageTextureResource resource(const ImageUploadBatch& upload, std::uint64_t frame) {
  return {.descriptor = upload.image, .format = VK_FORMAT_R8G8B8A8_UNORM,
      .extent = {1, 1}, .last_used_frame = frame};
}
int replacement_and_invalidation() {
  std::vector<VulkanRetainedImage> retained;
  std::vector<ImageUploadBatch> uploads{batch(1), batch(2)};
  const auto* payload = uploads[0].rgba.data();
  vulkan_retain_image_uploads(retained, uploads, 7);
  if (retained.size() != 2 || retained[0].upload.rgba.data() != payload || retained[0].last_used_frame != 7)
    return 1; // Ownership moved, not another per-frame byte copy.
  std::vector<ImageUploadBatch> replacements{batch(1, 2), batch(1, 3)};
  const auto* final_payload = replacements.back().rgba.data();
  vulkan_retain_image_uploads(retained, replacements, 9);
  if (retained.size() != 2 || retained[0].upload.rgba != std::vector<std::uint8_t>{3, 0, 0, 255} ||
      retained[0].upload.rgba.data() != final_payload || retained[0].last_used_frame != 9 ||
      retained[1].last_used_frame != 7) return 2;
  const std::array<ImageAssetId, 4> invalidations{{{0}, {99}, {1}, {1}}};
  vulkan_invalidate_retained_images(retained, invalidations);
  if (retained.size() != 1 || retained[0].upload.image.id.value != 2) return 3;
  const ImageAssetId last{2};
  vulkan_invalidate_retained_images(retained, std::span(&last, 1));
  return retained.empty() ? 0 : 4;
}
int allocation_prune() {
  std::vector<VulkanRetainedImage> retained{{batch(1), 3}, {batch(2), 4}, {batch(3), 5}};
  VulkanImageTextureResources textures;
  textures.textures = {resource(retained[0].upload, 70), resource(retained[1].upload, 80)};
  textures.textures[0].descriptor.logical_size = {300, 200}; // Not native allocation identity.
  textures.textures[1].extent.width = 2; // Same ID, different allocation cannot retain old pixels.
  vulkan_prune_retained_images(retained, textures);
  return retained.size() == 1 && retained[0].upload.image.id.value == 1 &&
      retained[0].last_used_frame == 70 ? 0 : 5;
}
int idle_prune() {
  std::vector<VulkanRetainedImage> retained{{batch(1), 10}};
  VulkanImageTextureResources textures;
  textures.textures = {resource(retained[0].upload, 10)};
  textures.frame_index = 130; // Exactly 120 idle frames are retained.
  if (vulkan_evict_idle_image_texture_resources(VK_NULL_HANDLE, textures) != 0) return 6;
  vulkan_prune_retained_images(retained, textures);
  if (retained.size() != 1 || retained[0].last_used_frame != 10) return 7;
  textures.frame_index = 131;
  if (vulkan_evict_idle_image_texture_resources(VK_NULL_HANDLE, textures) != 1) return 8;
  vulkan_prune_retained_images(retained, textures);
  if (!retained.empty()) return 9;
  retained = {{batch(4), 10}};
  textures.textures = {resource(retained[0].upload, 10)};
  textures.frame_index = std::numeric_limits<std::uint64_t>::max();
  vulkan_begin_image_texture_cache_frame(textures);
  vulkan_prune_retained_images(retained, textures);
  return retained.size() == 1 && retained[0].last_used_frame == 1 ? 0 : 10;
}
}
int main() {
  if (const auto result = replacement_and_invalidation()) return result;
  if (const auto result = allocation_prune()) return result;
  return idle_prune();
}
