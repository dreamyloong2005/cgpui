#include "vulkan_glyph_atlas_uploads_internal.hpp"

#include "vulkan_device_internal.hpp"

#include <algorithm>
#include <cstring>

namespace cgpui {
namespace {

constexpr std::size_t buffer_copy_alignment = 4;

std::size_t align_buffer_offset(std::size_t value) {
  return (value + buffer_copy_alignment - 1) &
         ~(buffer_copy_alignment - 1);
}

void destroy_staging_upload(
    VkDevice device,
    VulkanGlyphAtlasStagingUpload& upload) {
  if (upload.buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, upload.buffer, nullptr);
    upload.buffer = VK_NULL_HANDLE;
  }
  if (upload.memory != VK_NULL_HANDLE) {
    vkFreeMemory(device, upload.memory, nullptr);
    upload.memory = VK_NULL_HANDLE;
  }
}

Result<VulkanGlyphAtlasStagingUpload> stage_upload(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const GlyphAtlasProductionUploadCommand& command,
    const GlyphAtlasUploadBatch& batch) {
  if (command.byte_offset > batch.alpha.size() ||
      command.byte_size > batch.alpha.size() - command.byte_offset ||
      command.first_upload_index > batch.uploads.size() ||
      command.upload_count >
          batch.uploads.size() - command.first_upload_index ||
      command.upload_count == 0 || command.byte_size == 0) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "invalid glyph atlas dirty upload range"));
  }

  std::size_t staging_size = 0;
  for (std::size_t index = 0; index < command.upload_count; ++index) {
    const GlyphAtlasUploadRegion& region =
        batch.uploads[command.first_upload_index + index];
    staging_size = align_buffer_offset(staging_size);
    if (region.byte_offset > batch.alpha.size() ||
        region.byte_size > batch.alpha.size() - region.byte_offset ||
        region.byte_size > std::numeric_limits<std::size_t>::max() -
                               staging_size) {
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "invalid glyph atlas upload region"));
    }
    staging_size += region.byte_size;
  }

  VulkanGlyphAtlasStagingUpload upload{
      .page_index = command.page_index,
      .byte_size = staging_size,
  };
  const VkBufferCreateInfo buffer_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = staging_size,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  if (auto result = require_vk_success(
          vkCreateBuffer(device, &buffer_info, nullptr, &upload.buffer),
          "vkCreateBuffer for glyph atlas staging failed");
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
          "vkAllocateMemory for glyph atlas staging failed");
      !result) {
    destroy_staging_upload(device, upload);
    return std::unexpected(result.error());
  }
  if (auto result = require_vk_success(
          vkBindBufferMemory(device, upload.buffer, upload.memory, 0),
          "vkBindBufferMemory for glyph atlas staging failed");
      !result) {
    destroy_staging_upload(device, upload);
    return std::unexpected(result.error());
  }

  void* mapped = nullptr;
  if (auto result = require_vk_success(
          vkMapMemory(device, upload.memory, 0, staging_size, 0, &mapped),
          "vkMapMemory for glyph atlas staging failed");
      !result) {
    destroy_staging_upload(device, upload);
    return std::unexpected(result.error());
  }
  upload.copy_regions.reserve(command.upload_count);
  std::size_t buffer_offset = 0;
  for (std::size_t index = 0; index < command.upload_count; ++index) {
    const GlyphAtlasUploadRegion& region =
        batch.uploads[command.first_upload_index + index];
    buffer_offset = align_buffer_offset(buffer_offset);
    std::memcpy(
        static_cast<std::byte*>(mapped) + buffer_offset,
        batch.alpha.data() + region.byte_offset,
        region.byte_size);
    upload.copy_regions.push_back(vulkan_glyph_atlas_buffer_image_copy(
        region,
        buffer_offset));
    buffer_offset += region.byte_size;
  }
  vkUnmapMemory(device, upload.memory);
  return upload;
}

} // namespace

VkBufferImageCopy vulkan_glyph_atlas_buffer_image_copy(
    const GlyphAtlasUploadRegion& upload,
    std::size_t buffer_offset) {
  return VkBufferImageCopy{
      .bufferOffset = buffer_offset,
      .bufferRowLength = upload.stride == upload.width ? 0U : upload.stride,
      .bufferImageHeight = 0,
      .imageSubresource =
          VkImageSubresourceLayers{
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .imageOffset =
          VkOffset3D{
              .x = static_cast<std::int32_t>(upload.atlas_bounds.origin.x),
              .y = static_cast<std::int32_t>(upload.atlas_bounds.origin.y),
              .z = 0,
          },
      .imageExtent =
          VkExtent3D{
              .width = upload.width,
              .height = upload.height,
              .depth = 1,
          },
  };
}

Result<void> vulkan_stage_glyph_atlas_uploads(
    VkPhysicalDevice physical_device,
    VkDevice device,
    const GlyphAtlasProductionPlan& plan,
    std::span<const GlyphAtlasUploadBatch> batches,
    VulkanGlyphAtlasUploadResources& resources) {
  for (const GlyphAtlasProductionUploadCommand& command :
       plan.upload_commands) {
    const auto batch = std::ranges::find_if(
        batches,
        [&](const GlyphAtlasUploadBatch& candidate) {
          return candidate.image.page_index == command.page_index;
        });
    if (batch == batches.end()) {
      vulkan_destroy_glyph_atlas_upload_resources(device, resources);
      return std::unexpected(vulkan_error(
          ErrorCode::renderer_initialization_failed,
          "glyph atlas upload batch is missing"));
    }
    auto staged = stage_upload(physical_device, device, command, *batch);
    if (!staged) {
      vulkan_destroy_glyph_atlas_upload_resources(device, resources);
      return std::unexpected(staged.error());
    }
    resources.uploads.push_back(std::move(*staged));
  }
  return {};
}

void vulkan_destroy_glyph_atlas_upload_resources(
    VkDevice device,
    VulkanGlyphAtlasUploadResources& resources) {
  for (VulkanGlyphAtlasStagingUpload& upload : resources.uploads) {
    destroy_staging_upload(device, upload);
  }
  resources.uploads.clear();
}

} // namespace cgpui
