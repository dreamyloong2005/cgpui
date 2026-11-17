#include "vulkan_device_pixels_internal.hpp"

#include <cmath>

namespace cgpui {
namespace {

// Invalid or unit scales leave the draw untouched.
bool scales(DpiScale scale) {
  return std::isfinite(scale.value) && scale.value > 0.0F && scale.value != 1.0F;
}

BorderRadii scaled(BorderRadii radii, float s) {
  return BorderRadii::corners(radii.top_left * s, radii.top_right * s,
                              radii.bottom_right * s, radii.bottom_left * s);
}

Rect scaled(Rect rect, float s) {
  return Rect{
      .origin = {.x = rect.origin.x * s, .y = rect.origin.y * s},
      .size = {.width = rect.size.width * s, .height = rect.size.height * s}};
}

// A uniform scale commutes with the linear part of an affine transform, so
// S * T * p == T' * (S * p) where T' only has its translation scaled.
void scale_metadata(PaintMetadata& metadata, float s) {
  metadata.transform.translate_x *= s;
  metadata.transform.translate_y *= s;
}

template <typename Draw>
void scale_common(Draw& draw, float s) {
  if (draw.clip_rect.has_value()) draw.clip_rect = scaled(*draw.clip_rect, s);
  for (Rect& clip : draw.clip_stack.clips) clip = scaled(clip, s);
  if (draw.clip_stack.current_clip_rect.has_value()) {
    draw.clip_stack.current_clip_rect =
        scaled(*draw.clip_stack.current_clip_rect, s);
  }
  for (PaintMetadata& entry : draw.composition_stack.entries) {
    scale_metadata(entry, s);
  }
  scale_metadata(draw.composition_stack.current_metadata, s);
  scale_metadata(draw.metadata, s);
}

} // namespace

SolidRect vulkan_to_device_pixels(SolidRect draw, DpiScale scale) {
  if (!scales(scale)) return draw;
  draw.rect = scaled(draw.rect, scale.value);
  scale_common(draw, scale.value);
  return draw;
}

RoundedRectDraw vulkan_to_device_pixels(RoundedRectDraw draw, DpiScale scale) {
  if (!scales(scale)) return draw;
  const float s = scale.value;
  draw.rect = scaled(draw.rect, s);
  draw.radius = scaled(draw.radius, s);
  draw.border_width *= s;
  scale_common(draw, s);
  return draw;
}

TextDraw vulkan_to_device_pixels(TextDraw draw, DpiScale scale) {
  if (!scales(scale)) return draw;
  draw.bounds = scaled(draw.bounds, scale.value);
  scale_common(draw, scale.value);
  return draw;
}

TextSelectionDraw vulkan_to_device_pixels(
    TextSelectionDraw draw, DpiScale scale) {
  if (!scales(scale)) return draw;
  draw.rect = scaled(draw.rect, scale.value);
  scale_common(draw, scale.value);
  return draw;
}

TextCaretDraw vulkan_to_device_pixels(TextCaretDraw draw, DpiScale scale) {
  if (!scales(scale)) return draw;
  draw.rect = scaled(draw.rect, scale.value);
  scale_common(draw, scale.value);
  return draw;
}

// source_rect is in image space and is not a screen coordinate.
ImageDraw vulkan_to_device_pixels(ImageDraw draw, DpiScale scale) {
  if (!scales(scale)) return draw;
  draw.bounds = scaled(draw.bounds, scale.value);
  scale_common(draw, scale.value);
  return draw;
}

// Offset, blur and spread are lengths, so they scale with the rect.
BoxShadowDraw vulkan_to_device_pixels(BoxShadowDraw draw, DpiScale scale) {
  if (!scales(scale)) return draw;
  const float s = scale.value;
  draw.rect = scaled(draw.rect, s);
  draw.radius = scaled(draw.radius, s);
  draw.shadow.offset = {.x = draw.shadow.offset.x * s,
                        .y = draw.shadow.offset.y * s};
  draw.shadow.blur_radius *= s;
  draw.shadow.spread_radius *= s;
  scale_common(draw, s);
  return draw;
}

// The blur radius is a length (standard deviation) and scales too.
BackdropBlurDraw vulkan_to_device_pixels(BackdropBlurDraw draw, DpiScale scale) {
  if (!scales(scale)) return draw;
  draw.rect = scaled(draw.rect, scale.value);
  draw.radius = scaled(draw.radius, scale.value);
  draw.blur_radius *= scale.value;
  scale_common(draw, scale.value);
  return draw;
}

} // namespace cgpui
