#pragma once

#include "vulkan_frame_draw_order_internal.hpp"
#include "vulkan_rounded_rect_draw_recording_internal.hpp"
#include "vulkan_text_draw_recording_internal.hpp"

namespace cgpui {

void vulkan_record_frame_draws(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    const VulkanRoundedRectPipelineResources& rounded_rect_pipeline_resources,
    const VulkanRoundedRectBufferResources& solid_rect_buffers,
    const VulkanRoundedRectBufferResources& rounded_rect_buffers,
    const VulkanTextPipelineResources& text_pipeline_resources,
    const VulkanTextVertexBufferResources& text_vertex_buffer,
    std::span<const VulkanTextDrawCommand> text_commands,
    std::span<const VulkanFrameDrawOrderEntry> draw_order);

} // namespace cgpui
