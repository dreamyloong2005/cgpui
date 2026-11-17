#pragma once

#include "cgpui/renderer/renderer_commands.hpp"

namespace cgpui {

// RenderFrame draws arrive in logical pixels; the swapchain is in device
// pixels (framebuffer = logical size x DpiScale). Each draw is scaled once
// when the frame records it: rects, radii, border widths, clip rects, clip
// stacks and transform translations. TextDraw glyph `device_*` fields are
// already device pixels (laid out at TextDraw::scale) and are kept.
[[nodiscard]] SolidRect vulkan_to_device_pixels(SolidRect draw, DpiScale scale);
[[nodiscard]] RoundedRectDraw vulkan_to_device_pixels(
    RoundedRectDraw draw, DpiScale scale);
[[nodiscard]] TextDraw vulkan_to_device_pixels(TextDraw draw, DpiScale scale);
[[nodiscard]] TextSelectionDraw vulkan_to_device_pixels(
    TextSelectionDraw draw, DpiScale scale);
[[nodiscard]] TextCaretDraw vulkan_to_device_pixels(
    TextCaretDraw draw, DpiScale scale);
[[nodiscard]] ImageDraw vulkan_to_device_pixels(ImageDraw draw, DpiScale scale);
[[nodiscard]] BoxShadowDraw vulkan_to_device_pixels(
    BoxShadowDraw draw, DpiScale scale);
[[nodiscard]] BackdropBlurDraw vulkan_to_device_pixels(
    BackdropBlurDraw draw, DpiScale scale);

} // namespace cgpui
