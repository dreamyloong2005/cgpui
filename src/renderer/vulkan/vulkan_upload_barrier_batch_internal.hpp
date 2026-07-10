#pragma once

#include "vulkan_internal.hpp"

namespace cgpui {

struct VulkanUploadImageBarrierRequest {
  VkImage image = VK_NULL_HANDLE;
  VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct VulkanUploadImageBarrierBatch {
  VkPipelineStageFlags source_stage_mask = 0;
  VkPipelineStageFlags destination_stage_mask = 0;
  std::vector<VkImageMemoryBarrier> barriers;
};

struct VulkanUploadBarrierWave {
  VulkanUploadImageBarrierBatch to_transfer;
  VulkanUploadImageBarrierBatch to_readable;
  std::vector<std::size_t> upload_indices;
};

struct VulkanUploadBarrierPlan {
  std::vector<VulkanUploadBarrierWave> waves;
};

[[nodiscard]] VkPipelineStageFlags vulkan_upload_layout_stage(
    VkImageLayout layout);
[[nodiscard]] VkImageMemoryBarrier vulkan_upload_image_barrier(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout);
[[nodiscard]] VulkanUploadBarrierPlan vulkan_plan_upload_image_barriers(
    std::span<const VulkanUploadImageBarrierRequest> requests);
void vulkan_record_upload_image_barrier_batch(
    VkCommandBuffer command_buffer,
    const VulkanUploadImageBarrierBatch& batch);

} // namespace cgpui
