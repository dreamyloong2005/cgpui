#pragma once

#include "vulkan_rounded_rect_buffers_internal.hpp"
#include "vulkan_rounded_rect_pipeline_internal.hpp"

namespace cgpui {

[[nodiscard]] Result<void> vulkan_validate_rounded_rect_draw_resources(
    const VulkanRoundedRectPipelineResources& pipeline_resources,
    const VulkanRoundedRectBufferResources& buffer_resources);
void vulkan_record_rounded_rect_draws(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanRoundedRectPipelineResources& pipeline_resources,
    const VulkanRoundedRectBufferResources& buffer_resources);

} // namespace cgpui
