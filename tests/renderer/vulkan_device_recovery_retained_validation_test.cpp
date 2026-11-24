#include "vulkan_image_retention_internal.hpp"
#include <array>

using namespace cgpui;
int main() {
  const ImageUploadBatch batch{.image = {.id = {3}, .pixel_width = 1, .pixel_height = 1,
      .stride = 4, .byte_size = 4}, .uploads = {{{3}, 1, 1, 4, 0, 4}}, .rgba = {255, 0, 0, 255}};
  const std::array<VulkanRetainedImage, 1> retained{{{batch, 19}}};
  std::array<ImageDraw, 1> draws{{{.bounds = {{0, 0}, {16, 16}}, .asset = batch.image}}};
  if (!vulkan_validate_retained_image_requests(retained, draws, {}, {})) return 1;
  draws[0].asset.logical_size = {300, 200};
  draws[0].asset.stride = 8;
  draws[0].asset.byte_size = 99; // Upload metadata is not native allocation identity.
  if (!vulkan_validate_retained_image_requests(retained, draws, {}, {})) return 2;
  draws[0].asset.pixel_width = 2;
  const auto mismatch = vulkan_validate_retained_image_requests(retained, draws, {}, {});
  if (mismatch || mismatch.error().code != ErrorCode::invalid_argument) return 3;
  const ImageAssetId invalidation{3}, unrelated_invalidation{4};
  if (!vulkan_validate_retained_image_requests(retained, draws, {}, std::span(&invalidation, 1)) ||
      vulkan_validate_retained_image_requests(retained, draws, {}, std::span(&unrelated_invalidation, 1))) return 4;
  std::array<ImageUploadBatch, 1> replacement{{batch}};
  replacement[0].image.pixel_width = 2; // General input validation separately checks upload bytes/stride.
  if (!vulkan_validate_retained_image_requests(retained, draws, replacement, {})) return 5;
  replacement[0].image.id = {4};
  if (vulkan_validate_retained_image_requests(retained, draws, replacement, {})) return 6;
  draws[0].asset.id = {4}; // Valid not-yet-resident pixels remain a draw skip, not a false identity conflict.
  if (!vulkan_validate_retained_image_requests(retained, draws, {}, {})) return 7;
  const std::array<ImageDraw, 2> mixed{{draws[0], {.bounds = {{0, 0}, {16, 16}}, .asset = batch.image}}};
  auto conflicting = mixed;
  conflicting[1].asset.pixel_height = 2;
  if (!vulkan_validate_retained_image_requests(retained, mixed, {}, {}) ||
      vulkan_validate_retained_image_requests(retained, conflicting, {}, {}) ||
      retained[0].last_used_frame != 19 || retained[0].upload.rgba != batch.rgba ||
      retained[0].upload.image.pixel_width != 1) return 8;
  return 0;
}
