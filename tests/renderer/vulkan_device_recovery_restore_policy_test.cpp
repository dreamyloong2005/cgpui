#include "vulkan_image_retention_internal.hpp"
#include <array>

using namespace cgpui;
int main() {
  const ImageUploadBatch batch{.image = {.id = {3}, .pixel_width = 1, .pixel_height = 1,
      .stride = 4, .byte_size = 4}, .uploads = {{{3}, 1, 1, 4, 0, 4}}, .rgba = {255, 0, 0, 255}};
  const std::array<VulkanRetainedImage, 1> retained{{{batch, 19}}};
  std::array<ImageUploadBatch, 1> incoming{{batch}};
  incoming[0].rgba = {0, 255, 0, 255};
  VulkanImageTextureResources textures;
  VulkanImageTextureUploadResources uploads;
  // Empty or fully replaced restoration work never touches a GPU/loader.
  // A retained image needing native storage requires a live replacement device.
  if (!vulkan_restore_image_resources(VK_NULL_HANDLE, VK_NULL_HANDLE, {}, incoming, textures) ||
      !vulkan_restore_image_resources(VK_NULL_HANDLE, VK_NULL_HANDLE, retained, incoming, textures) ||
      !vulkan_restore_image_uploads(VK_NULL_HANDLE, VK_NULL_HANDLE, retained, incoming, textures, uploads) ||
      !textures.textures.empty() || !uploads.uploads.empty()) return 1;
  // Missing/evicted and incompatible resources must never resurrect old uploads.
  if (!vulkan_restore_image_uploads(VK_NULL_HANDLE, VK_NULL_HANDLE, retained, {}, textures, uploads)) return 2;
  textures.textures.push_back({.descriptor = batch.image, .format = VK_FORMAT_R8G8B8A8_UNORM,
      .extent = {2, 1}, .last_used_frame = 120});
  if (!vulkan_restore_image_uploads(VK_NULL_HANDLE, VK_NULL_HANDLE, retained, {}, textures, uploads) ||
      !uploads.uploads.empty()) return 3;
  textures.textures[0].extent.width = 1;
  if (!vulkan_restore_image_uploads(VK_NULL_HANDLE, VK_NULL_HANDLE, retained, incoming, textures, uploads) ||
      !uploads.uploads.empty()) return 4;
  // With a matching old resource and no replacement, native restoration is
  // required: null devices must fail rather than pretending pixels were staged.
  if (vulkan_restore_image_uploads(VK_NULL_HANDLE, VK_NULL_HANDLE, retained, {}, textures, uploads) ||
      !uploads.uploads.empty()) return 5;
  if (vulkan_restore_image_resources(VK_NULL_HANDLE, VK_NULL_HANDLE, retained, {}, textures) ||
      textures.textures[0].last_used_frame != 120 || retained[0].last_used_frame != 19 ||
      retained[0].upload.rgba != batch.rgba) return 6;
  return 0;
}
