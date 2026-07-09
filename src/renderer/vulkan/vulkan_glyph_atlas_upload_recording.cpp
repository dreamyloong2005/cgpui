#include "vulkan_glyph_atlas_uploads_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

VkAccessFlags access_for_layout(VkImageLayout layout) {
  switch (layout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      return VK_ACCESS_TRANSFER_WRITE_BIT;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      return VK_ACCESS_SHADER_READ_BIT;
    default:
      return 0;
  }
}

VkPipelineStageFlags destination_stage(VkImageLayout layout) {
  return layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
             ? VK_PIPELINE_STAGE_TRANSFER_BIT
             : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
}

} // namespace

VkPipelineStageFlags vulkan_glyph_atlas_source_stage(VkImageLayout layout) {
  switch (layout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    default:
      return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  }
}

VkImageMemoryBarrier vulkan_glyph_atlas_image_barrier(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout) {
  return VkImageMemoryBarrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = access_for_layout(old_layout),
      .dstAccessMask = access_for_layout(new_layout),
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          VkImageSubresourceRange{
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };
}

Result<void> vulkan_record_glyph_atlas_uploads(
    VkCommandBuffer command_buffer,
    const VulkanGlyphAtlasResources& atlas_resources,
    const VulkanGlyphAtlasUploadResources& upload_resources) {
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

    const VkImageMemoryBarrier to_transfer =
        vulkan_glyph_atlas_image_barrier(
            page->image,
            page->layout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vkCmdPipelineBarrier(
        command_buffer,
        vulkan_glyph_atlas_source_stage(page->layout),
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &to_transfer);
    vkCmdCopyBufferToImage(
        command_buffer,
        upload.buffer,
        page->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<std::uint32_t>(upload.copy_regions.size()),
        upload.copy_regions.data());

    const VkImageMemoryBarrier to_readable =
        vulkan_glyph_atlas_image_barrier(
            page->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkCmdPipelineBarrier(
        command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        destination_stage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &to_readable);
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
