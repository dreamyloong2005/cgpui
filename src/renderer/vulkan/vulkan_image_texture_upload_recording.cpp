#include "vulkan_image_texture_uploads_internal.hpp"

#include "vulkan_upload_barrier_batch_internal.hpp"

namespace cgpui {

VkPipelineStageFlags vulkan_image_texture_upload_source_stage(
    VkImageLayout layout) {
  return vulkan_upload_layout_stage(layout);
}

VkImageMemoryBarrier vulkan_image_texture_upload_barrier(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout) {
  return vulkan_upload_image_barrier(image, old_layout, new_layout);
}

Result<void> vulkan_record_image_texture_uploads(
    VkCommandBuffer command_buffer,
    const VulkanImageTextureResources& texture_resources,
    const VulkanImageTextureUploadResources& upload_resources) {
  struct ResolvedUpload {
    const VulkanImageTextureStagingUpload* upload = nullptr;
    const VulkanImageTextureResource* texture = nullptr;
  };
  std::vector<ResolvedUpload> resolved_uploads;
  std::vector<VulkanUploadImageBarrierRequest> barrier_requests;
  resolved_uploads.reserve(upload_resources.uploads.size());
  barrier_requests.reserve(upload_resources.uploads.size());
  for (const VulkanImageTextureStagingUpload& upload :
       upload_resources.uploads) {
    const VulkanImageTextureResource* texture =
        vulkan_find_image_texture_resource(texture_resources, upload.asset_id);
    if (texture == nullptr) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "image texture upload resource is missing"));
    }
    resolved_uploads.push_back(
        ResolvedUpload{.upload = &upload, .texture = texture});
    barrier_requests.push_back(VulkanUploadImageBarrierRequest{
        .image = texture->image,
        .old_layout = texture->layout,
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
          resolved.texture->image,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          1,
          &resolved.upload->copy_region);
    }
    vulkan_record_upload_image_barrier_batch(
        command_buffer, wave.to_readable);
  }
  return {};
}

void vulkan_commit_image_texture_uploads(
    VulkanImageTextureResources& texture_resources,
    const VulkanImageTextureUploadResources& upload_resources) {
  for (const VulkanImageTextureStagingUpload& upload :
       upload_resources.uploads) {
    VulkanImageTextureResource* texture =
        vulkan_find_image_texture_resource(texture_resources, upload.asset_id);
    if (texture != nullptr) {
      texture->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
  }
}

} // namespace cgpui
