#include "vulkan_solid_rect_recording_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"
#include "vulkan_composition_opacity_internal.hpp"

namespace cgpui {
namespace {

[[nodiscard]] bool make_clear_rect(
    VkExtent2D extent,
    const SolidRect& solid_rect,
    VkClearRect& clear_rect) {
  if (solid_rect.rect.size.width <= 0.0F ||
      solid_rect.rect.size.height <= 0.0F) {
    return false;
  }

  const VulkanClipScissorResolution clip =
      vulkan_resolve_clip_stack_scissor(
          extent, solid_rect.clip_rect, solid_rect.clip_stack);
  if (!clip.visible) {
    return false;
  }
  const float clip_left = static_cast<float>(clip.scissor.offset.x);
  const float clip_top = static_cast<float>(clip.scissor.offset.y);
  const float clip_right =
      clip_left + static_cast<float>(clip.scissor.extent.width);
  const float clip_bottom =
      clip_top + static_cast<float>(clip.scissor.extent.height);
  float left = std::max(solid_rect.rect.origin.x, clip_left);
  float top = std::max(solid_rect.rect.origin.y, clip_top);
  float right = std::clamp(
      solid_rect.rect.origin.x + solid_rect.rect.size.width,
      clip_left,
      clip_right);
  float bottom = std::clamp(
      solid_rect.rect.origin.y + solid_rect.rect.size.height,
      clip_top,
      clip_bottom);

  if (right <= left || bottom <= top) {
    return false;
  }
  const auto pixel_left = static_cast<std::int32_t>(std::floor(left));
  const auto pixel_top = static_cast<std::int32_t>(std::floor(top));
  const auto pixel_right = static_cast<std::int32_t>(std::ceil(right));
  const auto pixel_bottom = static_cast<std::int32_t>(std::ceil(bottom));
  if (pixel_right <= pixel_left || pixel_bottom <= pixel_top) {
    return false;
  }

  clear_rect = VkClearRect{
      .rect =
          VkRect2D{
              .offset = VkOffset2D{.x = pixel_left, .y = pixel_top},
              .extent =
                  VkExtent2D{
                      .width =
                          static_cast<std::uint32_t>(pixel_right - pixel_left),
                      .height =
                          static_cast<std::uint32_t>(pixel_bottom - pixel_top),
                  },
          },
      .baseArrayLayer = 0,
      .layerCount = 1,
  };
  return true;
}

} // namespace

void vulkan_record_solid_rects(
    VkCommandBuffer command_buffer,
    VkExtent2D extent,
    std::span<const SolidRect> rects) {
  for (const SolidRect& rect : rects) {
    VkClearRect clear_rect{};
    if (!make_clear_rect(extent, rect, clear_rect)) {
      continue;
    }
    VkClearAttachment attachment{};
    const Color color = vulkan_apply_composed_opacity(
        rect.color, rect.metadata, rect.composition_stack);
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = 0;
    attachment.clearValue.color = VkClearColorValue{{
        color.r,
        color.g,
        color.b,
        color.a,
    }};
    vkCmdClearAttachments(command_buffer, 1, &attachment, 1, &clear_rect);
  }
}

} // namespace cgpui
