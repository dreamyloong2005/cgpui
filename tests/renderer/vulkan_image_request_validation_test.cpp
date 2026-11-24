#include "vulkan_image_request_validation_internal.hpp"
#include <limits>

using namespace cgpui;
int main() {
  const ImageAsset image{.id = {910}, .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {0, 255, 0, 255}}};
  const auto asset = describe_image_asset(image);
  std::vector<ImageDraw> draws{ImageDraw{.bounds = {{0, 0}, {16, 16}}, .asset = asset}};
  VulkanImageTextureResources installed;
  const auto validate = [&](std::span<const ImageUploadBatch> uploads = {}, std::span<const ImageAssetId> invalidations = {}) {
    return vulkan_validate_image_frame_requests(draws, uploads, invalidations, installed, 4096);
  };
  if (!validate()) return 1; // Valid absence and zero native handles are CPU-only.
  installed.textures.push_back({.descriptor = asset, .format = VK_FORMAT_R8G8B8A8_UNORM, .extent = {1, 1}});
  for (unsigned cached = 0; cached < 2; ++cached) {
    if (cached) installed.textures.clear();
    draws[0].metadata.transform.skew_y = std::numeric_limits<float>::quiet_NaN();
    if (validate() || validate().error().code != ErrorCode::invalid_argument) return 2;
    draws[0].metadata = {};
    draws[0].asset.pixel_width = 4097;
    if (validate()) return 3;
    draws[0].asset = asset;
  }
  installed.textures.push_back({.descriptor = asset, .format = VK_FORMAT_R8G8B8A8_UNORM, .extent = {1, 1}});
  draws[0].asset.stride = 0; draws[0].asset.byte_size = 0;
  if (!validate()) return 4; // Draw-only padding is not identity or readiness.
  draws[0].asset = asset; draws[0].asset.pixel_width = 2;
  if (validate()) return 5; // Installed mismatch without explicit replacement is invalid.
  const ImageAssetId id = asset.id;
  if (!validate({}, std::span{&id, 1})) return 6; // Invalidation removes old identity.
  auto replacement = image;
  replacement.bitmap.width = 2; replacement.bitmap.stride = 8; replacement.bitmap.pixels.resize(8, 255);
  auto uploads = vulkan_plan_image_uploads(std::span{&replacement, 1});
  if (!validate(uploads)) return 7; // Explicit compatible final upload may replace allocation.
  uploads[0].rgba.clear();
  if (validate(uploads)) return 8;
  draws[0].asset = asset;
  draws.push_back(draws[0]); draws[1].asset.pixel_width = 2;
  installed.textures.clear();
  if (validate()) return 9; // Conflicts remain errors when neither ID has pixels.
  draws.resize(1);
  uploads = vulkan_plan_image_uploads(std::span{&image, 1});
  if (!validate(uploads)) return 10;
  uploads.push_back(uploads.front());
  return validate(uploads) ? 11 : 0;
}
