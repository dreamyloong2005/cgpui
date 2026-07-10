#include "vulkan_frame_command_reuse_internal.hpp"

#include <algorithm>

namespace cgpui {
namespace {

bool same_rect(Rect lhs, Rect rhs) {
  return lhs.origin.x == rhs.origin.x && lhs.origin.y == rhs.origin.y &&
         lhs.size.width == rhs.size.width &&
         lhs.size.height == rhs.size.height;
}

bool same_optional_rect(
    const std::optional<Rect>& lhs,
    const std::optional<Rect>& rhs) {
  return lhs.has_value() == rhs.has_value() &&
         (!lhs.has_value() || same_rect(*lhs, *rhs));
}

bool same_draw_range(
    const VulkanRoundedRectDrawRange& lhs,
    const VulkanRoundedRectDrawRange& rhs) {
  return lhs.source_index == rhs.source_index &&
         lhs.first_vertex == rhs.first_vertex &&
         lhs.vertex_count == rhs.vertex_count &&
         lhs.first_index == rhs.first_index &&
         lhs.index_count == rhs.index_count &&
         same_optional_rect(lhs.clip_rect, rhs.clip_rect);
}

bool same_text_command(
    const VulkanTextDrawCommand& lhs,
    const VulkanTextDrawCommand& rhs) {
  return lhs.text_draw_index == rhs.text_draw_index &&
         lhs.descriptor_set == rhs.descriptor_set &&
         lhs.first_vertex == rhs.first_vertex &&
         lhs.vertex_count == rhs.vertex_count &&
         same_optional_rect(lhs.clip_rect, rhs.clip_rect);
}

bool same_image_command(
    const VulkanImageDrawCommand& lhs,
    const VulkanImageDrawCommand& rhs) {
  return lhs.image_draw_index == rhs.image_draw_index &&
         lhs.descriptor_set == rhs.descriptor_set &&
         lhs.first_vertex == rhs.first_vertex &&
         lhs.vertex_count == rhs.vertex_count &&
         same_optional_rect(lhs.clip_rect, rhs.clip_rect) &&
         lhs.clip_stack == rhs.clip_stack;
}

template <typename Stored, typename Current, typename Predicate>
bool same_records(
    const std::vector<Stored>& stored,
    std::span<const Current> current,
    Predicate predicate) {
  return stored.size() == current.size() &&
         std::equal(
             stored.begin(), stored.end(), current.begin(), predicate);
}

bool same_signature(
    const VulkanFrameCommandSignature& stored,
    const VulkanFrameCommandSignatureView& current) {
  return stored.render_pass == current.render_pass &&
         stored.framebuffer == current.framebuffer &&
         stored.extent.width == current.extent.width &&
         stored.extent.height == current.extent.height &&
         stored.clear_color.r == current.clear_color.r &&
         stored.clear_color.g == current.clear_color.g &&
         stored.clear_color.b == current.clear_color.b &&
         stored.clear_color.a == current.clear_color.a &&
         stored.rounded_rect_pipeline_layout ==
             current.rounded_rect_pipeline_layout &&
         stored.rounded_rect_pipeline == current.rounded_rect_pipeline &&
         stored.text_pipeline_layout == current.text_pipeline_layout &&
         stored.text_pipeline == current.text_pipeline &&
         stored.image_pipeline_layout == current.image_pipeline_layout &&
         stored.image_pipeline == current.image_pipeline &&
         stored.solid_vertex_buffer == current.solid_vertex_buffer &&
         stored.solid_index_buffer == current.solid_index_buffer &&
         stored.rounded_vertex_buffer == current.rounded_vertex_buffer &&
         stored.rounded_index_buffer == current.rounded_index_buffer &&
         stored.text_vertex_buffer == current.text_vertex_buffer &&
         stored.image_vertex_buffer == current.image_vertex_buffer &&
         same_records(stored.solid_draws, current.solid_draws, same_draw_range) &&
         same_records(
             stored.rounded_draws, current.rounded_draws, same_draw_range) &&
         same_records(
             stored.text_commands, current.text_commands, same_text_command) &&
         same_records(
             stored.image_commands,
             current.image_commands,
             same_image_command) &&
         same_records(
             stored.draw_order,
             current.draw_order,
             [](const VulkanFrameDrawOrderEntry& lhs,
                const VulkanFrameDrawOrderEntry& rhs) { return lhs == rhs; });
}

void copy_signature(
    VulkanFrameCommandSignature& destination,
    const VulkanFrameCommandSignatureView& source) {
  destination.render_pass = source.render_pass;
  destination.framebuffer = source.framebuffer;
  destination.extent = source.extent;
  destination.clear_color = source.clear_color;
  destination.rounded_rect_pipeline_layout =
      source.rounded_rect_pipeline_layout;
  destination.rounded_rect_pipeline = source.rounded_rect_pipeline;
  destination.text_pipeline_layout = source.text_pipeline_layout;
  destination.text_pipeline = source.text_pipeline;
  destination.image_pipeline_layout = source.image_pipeline_layout;
  destination.image_pipeline = source.image_pipeline;
  destination.solid_vertex_buffer = source.solid_vertex_buffer;
  destination.solid_index_buffer = source.solid_index_buffer;
  destination.rounded_vertex_buffer = source.rounded_vertex_buffer;
  destination.rounded_index_buffer = source.rounded_index_buffer;
  destination.text_vertex_buffer = source.text_vertex_buffer;
  destination.image_vertex_buffer = source.image_vertex_buffer;
  destination.solid_draws.assign(
      source.solid_draws.begin(), source.solid_draws.end());
  destination.rounded_draws.assign(
      source.rounded_draws.begin(), source.rounded_draws.end());
  destination.text_commands.assign(
      source.text_commands.begin(), source.text_commands.end());
  destination.image_commands.assign(
      source.image_commands.begin(), source.image_commands.end());
  destination.draw_order.assign(
      source.draw_order.begin(), source.draw_order.end());
}

} // namespace

VulkanFrameCommandReusePlan vulkan_plan_frame_command_reuse(
    const VulkanFrameCommandReuseState& state,
    const VulkanFrameCommandSignatureView& signature,
    bool has_pending_uploads,
    bool frame_pixel_capture) {
  if (frame_pixel_capture) {
    return VulkanFrameCommandReusePlan{
        .reason = VulkanFrameCommandReuseReason::frame_pixel_capture,
    };
  }
  if (has_pending_uploads) {
    return VulkanFrameCommandReusePlan{
        .reason = VulkanFrameCommandReuseReason::pending_uploads,
    };
  }
  if (!state.valid) {
    return {};
  }
  if (!same_signature(state.signature, signature)) {
    return VulkanFrameCommandReusePlan{
        .reason = VulkanFrameCommandReuseReason::signature_changed,
    };
  }
  return VulkanFrameCommandReusePlan{
      .action = VulkanFrameCommandReuseAction::reuse,
      .reason = VulkanFrameCommandReuseReason::exact_match,
  };
}

void vulkan_commit_frame_command_recording(
    VulkanFrameCommandReuseState& state,
    const VulkanFrameCommandSignatureView& signature,
    bool has_pending_uploads,
    bool frame_pixel_capture) {
  state.recording_count += 1;
  if (has_pending_uploads || frame_pixel_capture) {
    state.valid = false;
    return;
  }
  copy_signature(state.signature, signature);
  state.valid = true;
}

void vulkan_commit_frame_command_reuse(VulkanFrameCommandReuseState& state) {
  state.reuse_count += 1;
}

void vulkan_invalidate_frame_command_reuse(
    VulkanFrameCommandReuseState& state) {
  state.valid = false;
}

void vulkan_invalidate_frame_command_reuse(
    std::span<VulkanFrameCommandReuseState> states) {
  for (VulkanFrameCommandReuseState& state : states) {
    vulkan_invalidate_frame_command_reuse(state);
  }
}

} // namespace cgpui
