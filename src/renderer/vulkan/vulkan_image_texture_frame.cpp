#include "vulkan_internal.hpp"

#include "vulkan_image_texture_cache_internal.hpp"
#include "vulkan_image_texture_invalidation_internal.hpp"
#include "vulkan_image_upload_retirement_internal.hpp"
#include "vulkan_image_request_validation_internal.hpp"

namespace cgpui {
Result<void> VulkanRendererState::prepare_image_texture_frame(
    std::span<const ImageDraw> image_draws,
    std::span<ImageUploadBatch> image_uploads,
    std::span<const ImageAssetId> image_invalidations) {
  VkPhysicalDeviceProperties properties{};
  if (!image_draws.empty() || !image_uploads.empty()) {
    if (physical_device_ == VK_NULL_HANDLE) return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed, "image frame requires a physical device"));
    vkGetPhysicalDeviceProperties(physical_device_, &properties);
  }
  if (auto result = vulkan_validate_image_frame_requests(image_draws, image_uploads,
          image_invalidations, image_texture_resources_, properties.limits.maxImageDimension2D);
      !result) {
    return result;
  }

  if (image_restore_pending_) {
    if (auto result = vulkan_validate_retained_image_requests(retained_image_uploads_,
            image_draws, image_uploads, image_invalidations); !result) return result;
  }
  const auto retained_uploads = vulkan_prepare_image_upload_retirement(
      device_, image_draws, image_uploads, image_invalidations, image_texture_uploads_);
  vulkan_invalidate_retained_images(retained_image_uploads_, image_invalidations);
  if (image_restore_pending_) {
    // A prior restoration failure may have staged only a prefix. No such frame
    // was submitted; discard that prefix before staging the complete retry.
    vulkan_destroy_image_texture_upload_resources(device_, image_texture_uploads_);
    if (auto result = vulkan_restore_image_resources(physical_device_, device_,
            retained_image_uploads_, image_uploads, image_texture_resources_); !result) return result;
  }
  (void)vulkan_invalidate_image_texture_resources(
      device_, image_invalidations, image_texture_resources_);
  (void)vulkan_prepare_image_texture_cache_frame(
      device_, image_draws, image_uploads, image_texture_resources_, retained_uploads);
  vulkan_prune_retained_images(retained_image_uploads_, image_texture_resources_);
  if (auto result = vulkan_update_image_texture_resources(
          physical_device_,
          device_,
          image_draws,
          image_texture_resources_);
      !result) {
    vulkan_prune_retained_images(retained_image_uploads_, image_texture_resources_);
    return result;
  }
  for (const ImageUploadBatch& upload : image_uploads) {
    if (auto result = vulkan_ensure_image_texture_resource(
            physical_device_,
            device_,
            upload.image,
            image_texture_resources_);
        !result) {
      vulkan_prune_retained_images(retained_image_uploads_, image_texture_resources_);
      return result;
    }
  }
  vulkan_prune_retained_images(retained_image_uploads_, image_texture_resources_);
  if (image_restore_pending_) {
    if (auto result = vulkan_restore_image_uploads(physical_device_, device_,
            retained_image_uploads_, image_uploads, image_texture_resources_,
            image_texture_uploads_); !result) return result;
  }
  if (auto result = vulkan_stage_image_texture_uploads(
          physical_device_, device_, image_uploads, image_texture_uploads_);
      !result) {
    return result;
  }
  pending_frame_upload_bytes_ = vulkan_staged_image_byte_counts(image_texture_uploads_);
  vulkan_retain_image_uploads(retained_image_uploads_, image_uploads,
                             image_texture_resources_.frame_index);
  image_restore_pending_ = false;
  return vulkan_upload_image_vertex_buffer(
      physical_device_, device_, image_draws, image_vertex_buffer_);
}

void VulkanRendererState::commit_image_texture_frame() {
  vulkan_commit_image_texture_uploads(
      image_texture_resources_, image_texture_uploads_);
}

} // namespace cgpui
