#include "vulkan_clip_scissor_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"

namespace cgpui {

std::optional<Rect> vulkan_resolve_effective_clip_rect(
    std::optional<Rect> clip, const RendererClipStackRecord& stack) {
  return resolve_effective_clip_rect(clip, stack);
}

VulkanClipScissorResolution vulkan_resolve_clip_stack_scissor(
    VkExtent2D extent, std::optional<Rect> clip, const RendererClipStackRecord& stack) {
  const auto result = resolve_clip_stack_scissor({extent.width, extent.height}, clip, stack);
  return {.scissor = {.offset = {result.scissor.x, result.scissor.y},
                      .extent = {result.scissor.width, result.scissor.height}},
          .visible = result.visible,
          .clipped = result.clipped};
}

} // namespace cgpui
