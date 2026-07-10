#pragma once

#include "vulkan_platform_internal.hpp"

#include <optional>

namespace cgpui {

struct VulkanClipScissorResolution {
  VkRect2D scissor{};
  bool visible = false;
  bool clipped = false;
};

[[nodiscard]] std::optional<Rect> vulkan_resolve_effective_clip_rect(
    std::optional<Rect> clip_rect,
    const RendererClipStackRecord& clip_stack);
[[nodiscard]] VulkanClipScissorResolution vulkan_resolve_clip_stack_scissor(
    VkExtent2D extent,
    std::optional<Rect> clip_rect,
    const RendererClipStackRecord& clip_stack);

} // namespace cgpui
