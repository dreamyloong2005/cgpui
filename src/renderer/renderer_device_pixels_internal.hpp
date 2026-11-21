#pragma once

#include "cgpui/renderer/renderer_commands.hpp"

namespace cgpui {

// RenderFrame draws arrive in logical pixels; the Vulkan swapchain and the
// Metal drawable are in device pixels (framebuffer = logical size x
// DpiScale). Each draw is scaled once
// when the frame records it: rects, radii, border widths, clip rects, clip
// stacks and transform translations. TextDraw glyph `device_*` fields are
// already device pixels (laid out at TextDraw::scale) and are kept.
[[nodiscard]] SolidRect renderer_to_device_pixels(SolidRect draw, DpiScale scale);
[[nodiscard]] RoundedRectDraw renderer_to_device_pixels(
    RoundedRectDraw draw, DpiScale scale);
[[nodiscard]] TextDraw renderer_to_device_pixels(TextDraw draw, DpiScale scale);
[[nodiscard]] TextSelectionDraw renderer_to_device_pixels(
    TextSelectionDraw draw, DpiScale scale);
[[nodiscard]] TextCaretDraw renderer_to_device_pixels(
    TextCaretDraw draw, DpiScale scale);
[[nodiscard]] ImageDraw renderer_to_device_pixels(ImageDraw draw, DpiScale scale);
[[nodiscard]] BoxShadowDraw renderer_to_device_pixels(
    BoxShadowDraw draw, DpiScale scale);
[[nodiscard]] BackdropBlurDraw renderer_to_device_pixels(
    BackdropBlurDraw draw, DpiScale scale);

} // namespace cgpui
