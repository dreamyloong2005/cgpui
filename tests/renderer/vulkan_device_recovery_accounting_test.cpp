#include "vulkan_image_retention_internal.hpp"
#include <limits>

using namespace cgpui;
int main() {
  const auto maximum = std::numeric_limits<std::size_t>::max();
  RendererFrameDiagnosticSnapshot snapshot{.retained_image_byte_count = maximum,
      .retained_image_byte_count_saturated = true};
  vulkan_report_retained_images({}, snapshot);
  if (snapshot.retained_image_byte_count != std::optional<std::size_t>{0} ||
      snapshot.retained_image_byte_count_saturated) return 1;
  std::vector<VulkanRetainedImage> retained(2);
  retained[0].upload.image.byte_size = maximum; // Descriptor claims are not allocated CPU bytes.
  retained[1].upload.image.byte_size = maximum;
  retained[0].upload.rgba = {1, 2, 3, 4};
  retained[1].upload.rgba = {5, 6, 7, 8, 9, 10, 11, 12};
  vulkan_report_retained_images(retained, snapshot);
  if (snapshot.retained_image_byte_count != std::optional<std::size_t>{12} ||
      snapshot.retained_image_byte_count_saturated) return 2;
  const auto historical = snapshot;
  retained.clear();
  vulkan_report_retained_images(retained, snapshot);
  if (snapshot.retained_image_byte_count != std::optional<std::size_t>{0} ||
      historical.retained_image_byte_count != std::optional<std::size_t>{12}) return 3;
  // Staging metadata permits safe overflow-boundary inputs without allocating
  // impossibly large vectors or forging std::vector internals/native buffers.
  VulkanImageTextureUploadResources staged;
  staged.uploads = {{.byte_size = maximum - 4}, {.byte_size = 4}};
  auto counts = vulkan_staged_image_byte_counts(staged);
  if (counts.image_byte_count != maximum || counts.total_byte_count != maximum ||
      counts.saturated || counts.glyph_atlas_byte_count) return 4;
  staged.uploads.push_back({.byte_size = 1});
  counts = vulkan_staged_image_byte_counts(staged);
  if (counts.image_byte_count != maximum || counts.total_byte_count != maximum || !counts.saturated) return 5;
  staged.uploads.push_back({.byte_size = maximum});
  counts = vulkan_staged_image_byte_counts(staged);
  if (counts.image_byte_count != maximum || !counts.saturated) return 6;
  staged.uploads.clear();
  counts = vulkan_staged_image_byte_counts(staged);
  return !counts.image_byte_count && !counts.total_byte_count && !counts.saturated ? 0 : 7;
}
