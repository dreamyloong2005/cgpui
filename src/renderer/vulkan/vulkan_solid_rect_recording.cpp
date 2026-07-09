#include "vulkan_solid_rect_recording_internal.hpp"

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

  const float framebuffer_width = static_cast<float>(extent.width);
  const float framebuffer_height = static_cast<float>(extent.height);
  float left = std::clamp(solid_rect.rect.origin.x, 0.0F, framebuffer_width);
  float top = std::clamp(solid_rect.rect.origin.y, 0.0F, framebuffer_height);
  float right = std::clamp(
      solid_rect.rect.origin.x + solid_rect.rect.size.width,
      0.0F,
      framebuffer_width);
  float bottom = std::clamp(
      solid_rect.rect.origin.y + solid_rect.rect.size.height,
      0.0F,
      framebuffer_height);

  if (solid_rect.clip_rect.has_value()) {
    const Rect& clip = *solid_rect.clip_rect;
    const float clip_left =
        std::clamp(clip.origin.x, 0.0F, framebuffer_width);
    const float clip_top = std::clamp(clip.origin.y, 0.0F, framebuffer_height);
    const float clip_right = std::clamp(
        clip.origin.x + clip.size.width, 0.0F, framebuffer_width);
    const float clip_bottom = std::clamp(
        clip.origin.y + clip.size.height, 0.0F, framebuffer_height);
    left = std::max(left, clip_left);
    top = std::max(top, clip_top);
    right = std::min(right, clip_right);
    bottom = std::min(bottom, clip_bottom);
  }

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
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = 0;
    attachment.clearValue.color = VkClearColorValue{{
        rect.color.r,
        rect.color.g,
        rect.color.b,
        rect.color.a,
    }};
    vkCmdClearAttachments(command_buffer, 1, &attachment, 1, &clear_rect);
  }
}

} // namespace cgpui
