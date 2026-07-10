#include "vulkan_glyph_atlas_uploads_internal.hpp"

#include "vulkan_upload_barrier_batch_internal.hpp"

#include <algorithm>

namespace cgpui {

VkPipelineStageFlags vulkan_glyph_atlas_source_stage(VkImageLayout layout) {
  return vulkan_upload_layout_stage(layout);
}

VkImageMemoryBarrier vulkan_glyph_atlas_image_barrier(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout) {
  return vulkan_upload_image_barrier(image, old_layout, new_layout);
}

Result<void> vulkan_record_glyph_atlas_uploads(
    VkCommandBuffer command_buffer,
    const VulkanGlyphAtlasResources& atlas_resources,
    const VulkanGlyphAtlasUploadResources& upload_resources) {
  struct ResolvedUpload {
    const VulkanGlyphAtlasStagingUpload* upload = nullptr;
    const VulkanGlyphAtlasPageResource* page = nullptr;
  };
  std::vector<ResolvedUpload> resolved_uploads;
  std::vector<VulkanUploadImageBarrierRequest> barrier_requests;
  resolved_uploads.reserve(upload_resources.uploads.size());
  barrier_requests.reserve(upload_resources.uploads.size());
  for (const VulkanGlyphAtlasStagingUpload& upload :
       upload_resources.uploads) {
    const auto page = std::ranges::find_if(
        atlas_resources.pages,
        [&](const VulkanGlyphAtlasPageResource& candidate) {
          return candidate.page_index == upload.page_index;
        });
    if (page == atlas_resources.pages.end()) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "glyph atlas upload page resource is missing"));
    }
    resolved_uploads.push_back(
        ResolvedUpload{.upload = &upload, .page = &*page});
    barrier_requests.push_back(VulkanUploadImageBarrierRequest{
        .image = page->image,
        .old_layout = page->layout,
    });
  }
  const VulkanUploadBarrierPlan barrier_plan =
      vulkan_plan_upload_image_barriers(barrier_requests);
  for (const VulkanUploadBarrierWave& wave : barrier_plan.waves) {
    vulkan_record_upload_image_barrier_batch(
        command_buffer, wave.to_transfer);
    for (std::size_t upload_index : wave.upload_indices) {
      const ResolvedUpload& resolved = resolved_uploads[upload_index];
      vkCmdCopyBufferToImage(
          command_buffer,
          resolved.upload->buffer,
          resolved.page->image,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          static_cast<std::uint32_t>(resolved.upload->copy_regions.size()),
          resolved.upload->copy_regions.data());
    }
    vulkan_record_upload_image_barrier_batch(
        command_buffer, wave.to_readable);
  }
  return {};
}

void vulkan_commit_glyph_atlas_uploads(
    VulkanGlyphAtlasResources& atlas_resources,
    const VulkanGlyphAtlasUploadResources& upload_resources) {
  for (const VulkanGlyphAtlasStagingUpload& upload :
       upload_resources.uploads) {
    const auto page = std::ranges::find_if(
        atlas_resources.pages,
        [&](const VulkanGlyphAtlasPageResource& candidate) {
          return candidate.page_index == upload.page_index;
        });
    if (page != atlas_resources.pages.end()) {
      page->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
  }
}

} // namespace cgpui
