#include "vulkan_frame_draw_recording_internal.hpp"

#include <optional>

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
    const VulkanImagePipelineResources& image_pipeline_resources,
    const VulkanImageVertexBufferResources& image_vertex_buffer,
    std::span<const VulkanImageDrawCommand> image_commands,
    std::span<const VulkanFrameDrawOrderEntry> draw_order) {
  VulkanFrameDrawOrderCursor cursor(
      draw_order,
      solid_rect_buffers.draws,
      rounded_rect_buffers.draws,
      text_commands,
      image_commands);
  std::optional<VulkanFrameDrawResourceKind> active_kind;
  while (const std::optional<VulkanResolvedFrameDraw> resolved = cursor.next()) {
    if (active_kind != resolved->resource_kind) {
      active_kind = resolved->resource_kind;
      if (*active_kind == VulkanFrameDrawResourceKind::solid_rect) {
        vulkan_bind_rounded_rect_draw_state(
            command_buffer,
            extent,
            rounded_rect_pipeline_resources,
            solid_rect_buffers);
      } else if (*active_kind == VulkanFrameDrawResourceKind::rounded_rect) {
        vulkan_bind_rounded_rect_draw_state(
            command_buffer,
            extent,
            rounded_rect_pipeline_resources,
            rounded_rect_buffers);
      } else if (*active_kind == VulkanFrameDrawResourceKind::text) {
        vulkan_bind_text_draw_state(
            command_buffer,
            extent,
            text_pipeline_resources,
            text_vertex_buffer);
      } else if (*active_kind == VulkanFrameDrawResourceKind::image) {
        vulkan_bind_image_draw_state(
            command_buffer,
            extent,
            image_pipeline_resources,
            image_vertex_buffer);
      }
    }
    if (*active_kind == VulkanFrameDrawResourceKind::solid_rect) {
      vulkan_record_rounded_rect_draw(
          command_buffer,
          extent,
          solid_rect_buffers.draws[resolved->resource_index]);
    } else if (*active_kind == VulkanFrameDrawResourceKind::rounded_rect) {
      vulkan_record_rounded_rect_draw(
          command_buffer,
          extent,
          rounded_rect_buffers.draws[resolved->resource_index]);
    } else if (*active_kind == VulkanFrameDrawResourceKind::text) {
      vulkan_record_text_draw(
          command_buffer,
          extent,
          text_pipeline_resources,
          text_commands[resolved->resource_index]);
    } else if (*active_kind == VulkanFrameDrawResourceKind::image) {
      vulkan_record_image_draw(
          command_buffer,
          extent,
          image_pipeline_resources,
          image_commands[resolved->resource_index]);
    }
  }
}

} // namespace cgpui
