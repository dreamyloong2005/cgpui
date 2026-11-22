#include "vulkan_image_upload_retirement_internal.hpp"
#include "vulkan_image_texture_cache_internal.hpp"

#include <iostream>

// Drives the same prepare/commit lifecycle as VulkanRendererState. Null
// native handles keep the lifetime test independent of a GPU and driver.
int main() {
  cgpui::VulkanImageTextureUploadResources pending;
  const cgpui::ImageAssetDescriptor descriptor{
      .id = {17}, .pixel_width = 2, .pixel_height = 2, .stride = 8, .byte_size = 16};
  pending.uploads.push_back({.asset_id = {17}, .byte_size = 16, .descriptor = descriptor});
  // Image 17 was staged, but acquisition requested a swapchain retry, so
  // there was no submit/commit. The next paint does not re-upload the asset.
  cgpui::vulkan_prepare_image_upload_retirement(VK_NULL_HANDLE, {}, {}, {}, pending);
  if (pending.uploads.size() != 1 || pending.uploads[0].asset_id.value != 17 ||
      pending.uploads[0].byte_size != 16) {
    std::cerr << "one-shot image upload was discarded before submission\n";
    return 1;
  }
  cgpui::VulkanImageTextureResources textures;
  textures.textures.push_back({.descriptor = descriptor});
  // Even repeated retries beyond the idle-cache horizon must preserve the
  // texture targeted by a one-shot upload, without a draw or repeated batch.
  for (int retry = 0; retry < 250; ++retry) {
    const auto retained = cgpui::vulkan_prepare_image_upload_retirement(
        VK_NULL_HANDLE, {}, {}, {}, pending);
    if (cgpui::vulkan_prepare_image_texture_cache_frame(
            VK_NULL_HANDLE, {}, {}, textures, retained) != 0 ||
        retained.size() != 1 || retained[0].value != 17) return 6;
  }
  cgpui::vulkan_commit_image_texture_uploads(textures, pending);
  if (!pending.submitted ||
      textures.textures[0].layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) return 2;
  cgpui::vulkan_prepare_image_upload_retirement(VK_NULL_HANDLE, {}, {}, {}, pending);
  if (!pending.uploads.empty() || pending.submitted) return 3;

  pending.uploads = {{.asset_id = {17}, .byte_size = 16}, {.asset_id = {18}, .byte_size = 32}};
  const cgpui::ImageAssetId invalidation{17};
  cgpui::vulkan_prepare_image_upload_retirement(
      VK_NULL_HANDLE, {}, {}, {&invalidation, 1}, pending);
  if (pending.uploads.size() != 1 || pending.uploads[0].asset_id.value != 18) return 4;
  const cgpui::ImageUploadBatch replacement{.image = {.id = {18}}};
  cgpui::vulkan_prepare_image_upload_retirement(
      VK_NULL_HANDLE, {}, {&replacement, 1}, {}, pending);
  if (!pending.uploads.empty()) return 5;

  pending.uploads.push_back({.asset_id = {17}, .byte_size = 16, .descriptor = descriptor});
  cgpui::ImageDraw draw{.asset = descriptor};
  draw.asset.logical_size = {200, 200}; // Logical-only changes preserve the allocation.
  cgpui::vulkan_prepare_image_upload_retirement(
      VK_NULL_HANDLE, {&draw, 1}, {}, {}, pending);
  if (pending.uploads.size() != 1) return 7;
  draw.asset.pixel_width = 1; // Reallocated texture must not receive an old 2x2 copy.
  cgpui::vulkan_prepare_image_upload_retirement(
      VK_NULL_HANDLE, {&draw, 1}, {}, {}, pending);
  if (!pending.uploads.empty()) return 8;
  return 0;
}
