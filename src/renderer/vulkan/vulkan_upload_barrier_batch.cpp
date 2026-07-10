#include "vulkan_upload_barrier_batch_internal.hpp"

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

bool batch_contains_image(
    const VulkanUploadImageBarrierBatch& batch,
    VkImage image) {
  return std::ranges::any_of(
      batch.barriers,
      [image](const VkImageMemoryBarrier& barrier) {
        return barrier.image == image;
      });
}

} // namespace

VkPipelineStageFlags vulkan_upload_layout_stage(VkImageLayout layout) {
  switch (layout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    default:
      return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  }
}

VkImageMemoryBarrier vulkan_upload_image_barrier(
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

VulkanUploadBarrierPlan vulkan_plan_upload_image_barriers(
    std::span<const VulkanUploadImageBarrierRequest> requests) {
  VulkanUploadBarrierPlan plan;
  std::vector<VkImage> seen_images;
  for (std::size_t index = 0; index < requests.size(); ++index) {
    const VulkanUploadImageBarrierRequest& request = requests[index];
    if (plan.waves.empty() ||
        batch_contains_image(plan.waves.back().to_transfer, request.image)) {
      plan.waves.emplace_back();
    }
    VulkanUploadBarrierWave& wave = plan.waves.back();
    const bool repeated =
        std::ranges::find(seen_images, request.image) != seen_images.end();
    const VkImageLayout old_layout =
        repeated ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                 : request.old_layout;

    wave.to_transfer.source_stage_mask |=
        vulkan_upload_layout_stage(old_layout);
    wave.to_transfer.destination_stage_mask =
        VK_PIPELINE_STAGE_TRANSFER_BIT;
    wave.to_transfer.barriers.push_back(vulkan_upload_image_barrier(
        request.image, old_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
    wave.to_readable.source_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    wave.to_readable.destination_stage_mask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    wave.to_readable.barriers.push_back(vulkan_upload_image_barrier(
        request.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
    wave.upload_indices.push_back(index);
    if (!repeated) {
      seen_images.push_back(request.image);
    }
  }
  return plan;
}

void vulkan_record_upload_image_barrier_batch(
    VkCommandBuffer command_buffer,
    const VulkanUploadImageBarrierBatch& batch) {
  if (batch.barriers.empty()) {
    return;
  }
  vkCmdPipelineBarrier(
      command_buffer,
      batch.source_stage_mask,
      batch.destination_stage_mask,
      0,
      0,
      nullptr,
      0,
      nullptr,
      static_cast<std::uint32_t>(batch.barriers.size()),
      batch.barriers.data());
}

} // namespace cgpui
