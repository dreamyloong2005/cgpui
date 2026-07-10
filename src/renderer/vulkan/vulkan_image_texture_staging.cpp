#include "vulkan_image_texture_uploads_internal.hpp"

#include "vulkan_device_internal.hpp"

#include <cstring>
#include <limits>

namespace cgpui {
namespace {

constexpr std::size_t rgba_bytes_per_pixel = 4;

void destroy_staging_upload(
    VkDevice device,
    VulkanImageTextureStagingUpload& upload) {
  if (device != VK_NULL_HANDLE) {
    if (upload.buffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(device, upload.buffer, nullptr);
    }
    if (upload.memory != VK_NULL_HANDLE) {
      vkFreeMemory(device, upload.memory, nullptr);
    }
  }
  upload = {};
}

Result<VulkanImageTextureStagingUpload> stage_upload(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const ImageUploadBatch& batch) {
  if (physical_device == VK_NULL_HANDLE || device == VK_NULL_HANDLE ||
      !vulkan_image_texture_upload_batch_valid(batch)) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "image texture staging requires a valid device and RGBA batch"));
  }

  VulkanImageTextureStagingUpload upload{
      .asset_id = batch.image.id,
      .byte_size = batch.rgba.size(),
      .copy_region = vulkan_image_texture_buffer_image_copy(
          batch.uploads.front(),
          batch.uploads.front().byte_offset),
  };
  const VkBufferCreateInfo buffer_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = upload.byte_size,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  if (auto result = require_vk_success(
          vkCreateBuffer(device, &buffer_info, nullptr, &upload.buffer),
          "vkCreateBuffer for image texture staging failed");
      !result) {
    return std::unexpected(result.error());
  }

  VkMemoryRequirements requirements{};
  vkGetBufferMemoryRequirements(device, upload.buffer, &requirements);
  auto memory_type = vulkan_find_memory_type(
      physical_device,
      requirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (!memory_type) {
    destroy_staging_upload(device, upload);
    return std::unexpected(memory_type.error());
  }
  const VkMemoryAllocateInfo allocation_info{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = requirements.size,
      .memoryTypeIndex = *memory_type,
  };
  if (auto result = require_vk_success(
          vkAllocateMemory(device, &allocation_info, nullptr, &upload.memory),
          "vkAllocateMemory for image texture staging failed");
      !result) {
    destroy_staging_upload(device, upload);
    return std::unexpected(result.error());
  }
  if (auto result = require_vk_success(
          vkBindBufferMemory(device, upload.buffer, upload.memory, 0),
          "vkBindBufferMemory for image texture staging failed");
      !result) {
    destroy_staging_upload(device, upload);
    return std::unexpected(result.error());
  }

  void* mapped = nullptr;
  if (auto result = require_vk_success(
          vkMapMemory(device, upload.memory, 0, upload.byte_size, 0, &mapped),
          "vkMapMemory for image texture staging failed");
      !result) {
    destroy_staging_upload(device, upload);
    return std::unexpected(result.error());
  }
  std::memcpy(mapped, batch.rgba.data(), batch.rgba.size());
  vkUnmapMemory(device, upload.memory);
  return upload;
}

} // namespace

bool vulkan_image_texture_upload_batch_valid(const ImageUploadBatch& batch) {
  if (!vulkan_image_texture_descriptor_valid(batch.image) ||
      batch.uploads.size() != 1 || batch.rgba.empty() ||
      batch.image.byte_size != batch.rgba.size()) {
    return false;
  }
  const ImageUploadRegion& upload = batch.uploads.front();
  if (upload.asset_id != batch.image.id ||
      upload.width != batch.image.pixel_width ||
      upload.height != batch.image.pixel_height ||
      upload.stride != batch.image.stride ||
      upload.stride % rgba_bytes_per_pixel != 0 ||
      upload.width >
          std::numeric_limits<std::uint32_t>::max() / rgba_bytes_per_pixel ||
      upload.stride < upload.width * rgba_bytes_per_pixel ||
      upload.byte_offset > batch.rgba.size() ||
      upload.byte_offset % rgba_bytes_per_pixel != 0 ||
      upload.byte_size > batch.rgba.size() - upload.byte_offset) {
    return false;
  }
  if (upload.height != 0 &&
      upload.stride > std::numeric_limits<std::size_t>::max() / upload.height) {
    return false;
  }
  const std::size_t required_bytes =
      static_cast<std::size_t>(upload.stride) * upload.height;
  return required_bytes <= upload.byte_size;
}

VkBufferImageCopy vulkan_image_texture_buffer_image_copy(
    const ImageUploadRegion& upload,
    std::size_t buffer_offset) {
  return VkBufferImageCopy{
      .bufferOffset = buffer_offset,
      .bufferRowLength =
          upload.stride == upload.width * rgba_bytes_per_pixel
              ? 0U
              : static_cast<std::uint32_t>(
                    upload.stride / rgba_bytes_per_pixel),
      .bufferImageHeight = 0,
      .imageSubresource =
          VkImageSubresourceLayers{
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .imageExtent =
          VkExtent3D{
              .width = upload.width,
              .height = upload.height,
              .depth = 1,
          },
  };
}

Result<void> vulkan_stage_image_texture_uploads(
    VkPhysicalDevice physical_device,
    VkDevice device,
    std::span<const ImageUploadBatch> batches,
    VulkanImageTextureUploadResources& resources) {
  for (const ImageUploadBatch& batch : batches) {
    auto staged = stage_upload(physical_device, device, batch);
    if (!staged) {
      vulkan_destroy_image_texture_upload_resources(device, resources);
      return std::unexpected(staged.error());
    }
    resources.uploads.push_back(std::move(*staged));
  }
  return {};
}

void vulkan_destroy_image_texture_upload_resources(
    VkDevice device,
    VulkanImageTextureUploadResources& resources) {
  for (VulkanImageTextureStagingUpload& upload : resources.uploads) {
    destroy_staging_upload(device, upload);
  }
  resources.uploads.clear();
}

} // namespace cgpui
