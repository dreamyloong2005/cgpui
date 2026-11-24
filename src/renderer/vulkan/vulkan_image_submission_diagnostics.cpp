#include "vulkan_image_submission_diagnostics_internal.hpp"
#include "vulkan_clip_scissor_internal.hpp"

namespace cgpui {
std::vector<std::size_t> vulkan_submitted_image_indices(
    VkExtent2D extent, std::span<const VulkanFrameDrawOrderEntry> draw_order,
    std::span<const VulkanImageDrawCommand> image_commands) {
  std::vector<std::size_t> result;
  // Use the recorder's selection cursor, not a second texture-readiness guess.
  VulkanFrameDrawOrderCursor cursor(draw_order, {}, {}, {}, image_commands);
  while (const auto selected = cursor.next()) {
    if (selected->resource_kind != VulkanFrameDrawResourceKind::image) continue;
    const auto& command = image_commands[selected->resource_index];
    if (vulkan_resolve_clip_stack_scissor(extent, command.clip_rect, command.clip_stack).visible)
      result.push_back(command.image_draw_index);
  }
  return result;
}
} // namespace cgpui
