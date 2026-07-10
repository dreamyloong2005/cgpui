#include "vulkan_clip_scissor_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {
namespace {

[[nodiscard]] Rect intersect_rects(Rect first, Rect second) {
  const float left = std::max(first.origin.x, second.origin.x);
  const float top = std::max(first.origin.y, second.origin.y);
  const float right = std::min(
      first.origin.x + first.size.width,
      second.origin.x + second.size.width);
  const float bottom = std::min(
      first.origin.y + first.size.height,
      second.origin.y + second.size.height);
  return Rect{
      .origin = {.x = left, .y = top},
      .size = {
          .width = std::max(0.0F, right - left),
          .height = std::max(0.0F, bottom - top),
      },
  };
}

void apply_clip(std::optional<Rect>& effective, Rect clip) {
  effective = effective.has_value() ? intersect_rects(*effective, clip) : clip;
}

} // namespace

std::optional<Rect> vulkan_resolve_effective_clip_rect(
    std::optional<Rect> clip_rect,
    const RendererClipStackRecord& clip_stack) {
  std::optional<Rect> effective;
  for (const Rect clip : clip_stack.clips) {
    apply_clip(effective, clip);
  }
  if (clip_stack.current_clip_rect.has_value()) {
    apply_clip(effective, *clip_stack.current_clip_rect);
  }
  if (clip_rect.has_value()) {
    apply_clip(effective, *clip_rect);
  }
  return effective;
}

VulkanClipScissorResolution vulkan_resolve_clip_stack_scissor(
    VkExtent2D extent,
    std::optional<Rect> clip_rect,
    const RendererClipStackRecord& clip_stack) {
  const std::optional<Rect> effective =
      vulkan_resolve_effective_clip_rect(clip_rect, clip_stack);
  const bool clipped = effective.has_value();
  if (extent.width == 0 || extent.height == 0) {
    return VulkanClipScissorResolution{.clipped = clipped};
  }

  const float framebuffer_width = static_cast<float>(extent.width);
  const float framebuffer_height = static_cast<float>(extent.height);
  float left = 0.0F;
  float top = 0.0F;
  float right = framebuffer_width;
  float bottom = framebuffer_height;
  if (effective.has_value()) {
    const Rect clip = *effective;
    if (!std::isfinite(clip.origin.x) || !std::isfinite(clip.origin.y) ||
        !std::isfinite(clip.size.width) || !std::isfinite(clip.size.height)) {
      return VulkanClipScissorResolution{.clipped = true};
    }
    left = std::clamp(clip.origin.x, 0.0F, framebuffer_width);
    top = std::clamp(clip.origin.y, 0.0F, framebuffer_height);
    right = std::clamp(
        clip.origin.x + clip.size.width, 0.0F, framebuffer_width);
    bottom = std::clamp(
        clip.origin.y + clip.size.height, 0.0F, framebuffer_height);
  }
  if (right <= left || bottom <= top) {
    return VulkanClipScissorResolution{.clipped = clipped};
  }

  const auto pixel_left = static_cast<std::int32_t>(std::floor(left));
  const auto pixel_top = static_cast<std::int32_t>(std::floor(top));
  const auto pixel_right = static_cast<std::int32_t>(std::ceil(right));
  const auto pixel_bottom = static_cast<std::int32_t>(std::ceil(bottom));
  return VulkanClipScissorResolution{
      .scissor =
          VkRect2D{
              .offset = {.x = pixel_left, .y = pixel_top},
              .extent = {
                  .width = static_cast<std::uint32_t>(pixel_right - pixel_left),
                  .height = static_cast<std::uint32_t>(pixel_bottom - pixel_top),
              },
          },
      .visible = pixel_right > pixel_left && pixel_bottom > pixel_top,
      .clipped = clipped,
  };
}

} // namespace cgpui
