#include "vulkan_internal.hpp"

#include "vulkan_image_texture_cache_internal.hpp"

namespace cgpui {
namespace {

bool same_allocation_identity(
    const ImageAssetDescriptor& lhs,
    const ImageAssetDescriptor& rhs) {
  return lhs.id == rhs.id && lhs.pixel_width == rhs.pixel_width &&
         lhs.pixel_height == rhs.pixel_height && lhs.format == rhs.format;
}

Result<void> validate_image_texture_frame_requests(
    std::span<const ImageDraw> image_draws,
    std::span<const ImageUploadBatch> image_uploads) {
  if (auto result =
          vulkan_validate_image_texture_resource_requests(image_draws);
      !result) {
    return result;
  }
  for (std::size_t index = 0; index < image_uploads.size(); ++index) {
    const ImageUploadBatch& upload = image_uploads[index];
    if (!vulkan_image_texture_upload_batch_valid(upload)) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "image texture frame contains an invalid upload batch"));
    }
    for (std::size_t prior = 0; prior < index; ++prior) {
      if (image_uploads[prior].image.id == upload.image.id) {
        return std::unexpected(vulkan_error(
            ErrorCode::renderer_initialization_failed,
            "image texture frame contains duplicate uploads"));
      }
    }
    for (const ImageDraw& draw : image_draws) {
      if (draw.asset.id == upload.image.id &&
          !same_allocation_identity(draw.asset, upload.image)) {
        return std::unexpected(vulkan_error(
            ErrorCode::renderer_initialization_failed,
            "image texture draw conflicts with its upload"));
      }
    }
  }
  return {};
}

} // namespace

Result<void> VulkanRendererState::prepare_image_texture_frame(
    std::span<const ImageDraw> image_draws,
    std::span<const ImageUploadBatch> image_uploads) {
  if (auto result =
          validate_image_texture_frame_requests(image_draws, image_uploads);
      !result) {
    return result;
  }

  vulkan_destroy_image_texture_upload_resources(
      device_, image_texture_uploads_);
  (void)vulkan_prepare_image_texture_cache_frame(
      device_, image_draws, image_uploads, image_texture_resources_);
  if (auto result = vulkan_update_image_texture_resources(
          physical_device_,
          device_,
          image_draws,
          image_texture_resources_);
      !result) {
    return result;
  }
  for (const ImageUploadBatch& upload : image_uploads) {
    if (auto result = vulkan_ensure_image_texture_resource(
            physical_device_,
            device_,
            upload.image,
            image_texture_resources_);
        !result) {
      return result;
    }
  }
  if (auto result = vulkan_stage_image_texture_uploads(
          physical_device_, device_, image_uploads, image_texture_uploads_);
      !result) {
    return result;
  }
  return vulkan_upload_image_vertex_buffer(
      physical_device_, device_, image_draws, image_vertex_buffer_);
}

void VulkanRendererState::commit_image_texture_frame() {
  vulkan_commit_image_texture_uploads(
      image_texture_resources_, image_texture_uploads_);
}

} // namespace cgpui
