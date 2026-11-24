#include "vulkan_image_retention_internal.hpp"

#include <limits>

namespace cgpui {

void vulkan_invalidate_retained_images(
    std::vector<VulkanRetainedImage>& retained, std::span<const ImageAssetId> invalidations) {
  std::erase_if(retained, [&](const auto& entry) {
    return std::ranges::find(invalidations, entry.upload.image.id) != invalidations.end();
  });
}

void vulkan_prune_retained_images(
    std::vector<VulkanRetainedImage>& retained, const VulkanImageTextureResources& resources) {
  std::erase_if(retained, [&](const VulkanRetainedImage& entry) {
    const auto* texture = vulkan_find_image_texture_resource(resources, entry.upload.image.id);
    return !texture || !vulkan_image_texture_resource_matches(*texture, entry.upload.image);
  });
  // erase_if/remove_if predicates must accept const elements without mutation.
  // Synchronize surviving ages separately from the deletion decision.
  for (auto& entry : retained)
    entry.last_used_frame = vulkan_find_image_texture_resource(resources, entry.upload.image.id)->last_used_frame;
}

void vulkan_retain_image_uploads(
    std::vector<VulkanRetainedImage>& retained, std::span<ImageUploadBatch> uploads,
    std::uint64_t frame_index) {
  for (auto& upload : uploads) {
    auto entry = std::ranges::find_if(retained, [&](const auto& candidate) {
      return candidate.upload.image.id == upload.image.id;
    });
    VulkanRetainedImage next{.upload = std::move(upload), .last_used_frame = frame_index};
    if (entry == retained.end()) retained.push_back(std::move(next));
    else *entry = std::move(next);
  }
}

void vulkan_report_retained_images(
    std::span<const VulkanRetainedImage> retained, RendererFrameDiagnosticSnapshot& snapshot) {
  snapshot.retained_image_byte_count = 0;
  snapshot.retained_image_byte_count_saturated = false;
  for (const auto& entry : retained) {
    auto& count = *snapshot.retained_image_byte_count;
    const auto maximum = std::numeric_limits<std::size_t>::max();
    if (entry.upload.rgba.size() > maximum - count) {
      count = maximum;
      snapshot.retained_image_byte_count_saturated = true;
    } else count += entry.upload.rgba.size();
  }
}

RendererUploadByteCounts vulkan_staged_image_byte_counts(
    const VulkanImageTextureUploadResources& uploads) {
  RendererUploadByteCounts counts;
  for (const auto& upload : uploads.uploads) {
    const auto maximum = std::numeric_limits<std::size_t>::max();
    if (upload.byte_size > maximum - counts.image_byte_count) {
      counts.image_byte_count = maximum;
      counts.saturated = true;
    } else counts.image_byte_count += upload.byte_size;
  }
  counts.total_byte_count = counts.image_byte_count;
  return counts;
}

} // namespace cgpui
