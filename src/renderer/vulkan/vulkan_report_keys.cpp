#include "vulkan_report_internal.hpp"

#include <cstddef>

namespace cgpui {

bool vulkan_same_rect(Rect lhs, Rect rhs) {
  return lhs.origin.x == rhs.origin.x && lhs.origin.y == rhs.origin.y &&
         lhs.size.width == rhs.size.width && lhs.size.height == rhs.size.height;
}

bool vulkan_same_clip_rect(
    const std::optional<Rect>& lhs,
    const std::optional<Rect>& rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }
  return !lhs.has_value() || vulkan_same_rect(*lhs, *rhs);
}

bool vulkan_same_clip_stack(
    const RendererClipStackRecord& lhs,
    const RendererClipStackRecord& rhs) {
  if (lhs.full_depth != rhs.full_depth || lhs.truncated != rhs.truncated ||
      !vulkan_same_clip_rect(lhs.current_clip_rect, rhs.current_clip_rect) ||
      lhs.clips.size() != rhs.clips.size()) {
    return false;
  }
  for (std::size_t index = 0; index < lhs.clips.size(); ++index) {
    if (!vulkan_same_rect(lhs.clips[index], rhs.clips[index])) {
      return false;
    }
  }
  return true;
}

bool vulkan_same_command_batch_key(
    const RendererCommandBatchKey& lhs,
    const RendererCommandBatchKey& rhs) {
  return lhs.primitive_kind == rhs.primitive_kind &&
         vulkan_same_clip_rect(lhs.clip_rect, rhs.clip_rect) &&
         vulkan_same_clip_stack(lhs.clip_stack, rhs.clip_stack) &&
         lhs.composition_stack == rhs.composition_stack &&
         lhs.metadata == rhs.metadata;
}

bool vulkan_same_submission_plan_key(
    const RendererSubmissionPlanKey& lhs,
    const RendererSubmissionPlanKey& rhs) {
  return lhs.primitive_kind == rhs.primitive_kind &&
         vulkan_same_clip_rect(lhs.clip_rect, rhs.clip_rect) &&
         vulkan_same_clip_stack(lhs.clip_stack, rhs.clip_stack) &&
         lhs.atlas_page_index == rhs.atlas_page_index;
}

} // namespace cgpui
