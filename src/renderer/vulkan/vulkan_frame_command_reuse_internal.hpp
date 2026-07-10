#pragma once

#include "vulkan_frame_draw_order_internal.hpp"

namespace cgpui {

enum class VulkanFrameCommandReuseAction {
  record,
  reuse,
};

enum class VulkanFrameCommandReuseReason {
  no_recording,
  pending_uploads,
  signature_changed,
  exact_match,
};

struct VulkanFrameCommandReusePlan {
  VulkanFrameCommandReuseAction action =
      VulkanFrameCommandReuseAction::record;
  VulkanFrameCommandReuseReason reason =
      VulkanFrameCommandReuseReason::no_recording;
};

struct VulkanFrameCommandSignatureView {
  VkRenderPass render_pass = VK_NULL_HANDLE;
  VkFramebuffer framebuffer = VK_NULL_HANDLE;
  VkExtent2D extent{};
  Color clear_color;
  VkPipelineLayout rounded_rect_pipeline_layout = VK_NULL_HANDLE;
  VkPipeline rounded_rect_pipeline = VK_NULL_HANDLE;
  VkPipelineLayout text_pipeline_layout = VK_NULL_HANDLE;
  VkPipeline text_pipeline = VK_NULL_HANDLE;
  VkPipelineLayout image_pipeline_layout = VK_NULL_HANDLE;
  VkPipeline image_pipeline = VK_NULL_HANDLE;
  VkBuffer solid_vertex_buffer = VK_NULL_HANDLE;
  VkBuffer solid_index_buffer = VK_NULL_HANDLE;
  VkBuffer rounded_vertex_buffer = VK_NULL_HANDLE;
  VkBuffer rounded_index_buffer = VK_NULL_HANDLE;
  VkBuffer text_vertex_buffer = VK_NULL_HANDLE;
  VkBuffer image_vertex_buffer = VK_NULL_HANDLE;
  std::span<const VulkanRoundedRectDrawRange> solid_draws;
  std::span<const VulkanRoundedRectDrawRange> rounded_draws;
  std::span<const VulkanTextDrawCommand> text_commands;
  std::span<const VulkanImageDrawCommand> image_commands;
  std::span<const VulkanFrameDrawOrderEntry> draw_order;
};

struct VulkanFrameCommandSignature {
  VkRenderPass render_pass = VK_NULL_HANDLE;
  VkFramebuffer framebuffer = VK_NULL_HANDLE;
  VkExtent2D extent{};
  Color clear_color;
  VkPipelineLayout rounded_rect_pipeline_layout = VK_NULL_HANDLE;
  VkPipeline rounded_rect_pipeline = VK_NULL_HANDLE;
  VkPipelineLayout text_pipeline_layout = VK_NULL_HANDLE;
  VkPipeline text_pipeline = VK_NULL_HANDLE;
  VkPipelineLayout image_pipeline_layout = VK_NULL_HANDLE;
  VkPipeline image_pipeline = VK_NULL_HANDLE;
  VkBuffer solid_vertex_buffer = VK_NULL_HANDLE;
  VkBuffer solid_index_buffer = VK_NULL_HANDLE;
  VkBuffer rounded_vertex_buffer = VK_NULL_HANDLE;
  VkBuffer rounded_index_buffer = VK_NULL_HANDLE;
  VkBuffer text_vertex_buffer = VK_NULL_HANDLE;
  VkBuffer image_vertex_buffer = VK_NULL_HANDLE;
  std::vector<VulkanRoundedRectDrawRange> solid_draws;
  std::vector<VulkanRoundedRectDrawRange> rounded_draws;
  std::vector<VulkanTextDrawCommand> text_commands;
  std::vector<VulkanImageDrawCommand> image_commands;
  std::vector<VulkanFrameDrawOrderEntry> draw_order;
};

struct VulkanFrameCommandReuseState {
  VulkanFrameCommandSignature signature;
  std::size_t recording_count = 0;
  std::size_t reuse_count = 0;
  bool valid = false;
};

struct VulkanFrameCommandRecordingResult {
  VulkanFrameCommandReuseAction action =
      VulkanFrameCommandReuseAction::record;
  VulkanFrameCommandReuseReason reason =
      VulkanFrameCommandReuseReason::no_recording;
};

[[nodiscard]] VulkanFrameCommandReusePlan vulkan_plan_frame_command_reuse(
    const VulkanFrameCommandReuseState& state,
    const VulkanFrameCommandSignatureView& signature,
    bool has_pending_uploads);
void vulkan_commit_frame_command_recording(
    VulkanFrameCommandReuseState& state,
    const VulkanFrameCommandSignatureView& signature,
    bool has_pending_uploads);
void vulkan_commit_frame_command_reuse(VulkanFrameCommandReuseState& state);
void vulkan_invalidate_frame_command_reuse(
    VulkanFrameCommandReuseState& state);
void vulkan_invalidate_frame_command_reuse(
    std::span<VulkanFrameCommandReuseState> states);

} // namespace cgpui
