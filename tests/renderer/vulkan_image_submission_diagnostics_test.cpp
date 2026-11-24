#include "vulkan_image_submission_diagnostics_internal.hpp"
#include "vulkan_frame_diagnostic_snapshot_internal.hpp"
#include "vulkan_frame_command_reuse_internal.hpp"
#include <array>

using namespace cgpui;
int main() {
  const std::array order{VulkanFrameDrawOrderEntry{RendererPrimitiveKind::image, 0},
      VulkanFrameDrawOrderEntry{RendererPrimitiveKind::image, 1},
      VulkanFrameDrawOrderEntry{RendererPrimitiveKind::image, 2}};
  std::vector<VulkanImageDrawCommand> commands{
      {.image_draw_index = 0, .vertex_count = 6},
      {.image_draw_index = 2, .vertex_count = 6, .clip_rect = Rect{{128, 128}, {1, 1}}},
      {.image_draw_index = 3, .vertex_count = 6}}; // Unselected command must not count.
  const auto selected = vulkan_submitted_image_indices({64, 64}, order, commands);
  if (selected != std::vector<std::size_t>{0}) return 1;
  const auto resources = vulkan_build_frame_diagnostic_resources(order, {}, {}, {}, selected, {});
  if (resources.planned_draws.image_count != 3 || resources.submitted_draws.image_count != 1 ||
      resources.submitted_command_count != 1 || resources.dropped_resources.resources.size() != 2 ||
      resources.dropped_resources.missing_submission_resource_count != 2) return 2;
  commands[1].clip_rect.reset();
  commands[1].clip_stack.current_clip_rect = Rect{{128, 128}, {1, 1}};
  if (vulkan_submitted_image_indices({64, 64}, order, commands) != selected ||
      !vulkan_submitted_image_indices({0, 0}, order, commands).empty()) return 3;
  VulkanFrameCommandRecordingResult recorded{.submitted_image_indices = selected};
  VulkanFrameCommandRecordingResult reused{.action = VulkanFrameCommandReuseAction::reuse,
      .reason = VulkanFrameCommandReuseReason::exact_match,
      .submitted_image_indices = vulkan_submitted_image_indices({64, 64}, order, commands)};
  commands.clear(); // Successful result owns evidence even when planning vector dies.
  if (recorded.submitted_image_indices != reused.submitted_image_indices || recorded.submitted_image_indices != selected) return 4;
  const auto missing = vulkan_build_frame_diagnostic_resources(order, {}, {}, {}, {}, {});
  return missing.submitted_draws.image_count == 0 && missing.dropped_resources.missing_submission_resource_count == 3 ? 0 : 5;
}
