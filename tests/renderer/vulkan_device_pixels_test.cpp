#include "renderer_device_pixels_internal.hpp"

#include <iostream>

namespace {

int fail(int code, const char* message) {
  std::cerr << "vulkan device pixels: " << message << '\n';
  return code;
}

bool same(cgpui::Rect rect, float x, float y, float width, float height) {
  return rect.origin.x == x && rect.origin.y == y &&
         rect.size.width == width && rect.size.height == height;
}

} // namespace

int main() {
  const cgpui::DpiScale scale{1.5F};
  cgpui::RendererClipStackRecord clips;
  clips.clips = {cgpui::Rect{{0, 0}, {100, 32}}};
  clips.full_depth = 1;
  clips.current_clip_rect = cgpui::Rect{{0, 0}, {100, 32}};
  const cgpui::SolidRect rect = cgpui::renderer_to_device_pixels(
      cgpui::SolidRect{
          .rect = {{330, 4}, {240, 24}},
          .clip_rect = cgpui::Rect{{10, 0}, {20, 10}},
          .clip_stack = clips,
          .metadata = {.transform = cgpui::AffineTransform{
                           .scale_x = 2, .translate_x = 8, .translate_y = 4}},
      },
      scale);
  if (!same(rect.rect, 495, 6, 360, 36)) {
    return fail(1, "rects must be scaled to device pixels");
  }
  if (!rect.clip_rect || !same(*rect.clip_rect, 15, 0, 30, 15) ||
      !same(rect.clip_stack.clips[0], 0, 0, 150, 48) ||
      !same(*rect.clip_stack.current_clip_rect, 0, 0, 150, 48)) {
    return fail(2, "clip rects and clip stacks must be scaled");
  }
  if (rect.metadata.transform.scale_x != 2 ||
      rect.metadata.transform.translate_x != 12 ||
      rect.metadata.transform.translate_y != 6) {
    return fail(3, "only transform translations scale");
  }
  const cgpui::RoundedRectDraw rounded = cgpui::renderer_to_device_pixels(
      cgpui::RoundedRectDraw{.rect = {{0, 0}, {10, 10}},
                             .radius = cgpui::BorderRadii::all(4),
                             .border_width = 2},
      scale);
  if (rounded.radius.top_left != 6 || rounded.radius.bottom_right != 6 ||
      rounded.border_width != 3) {
    return fail(4, "radii and border widths must be scaled");
  }
  cgpui::TextDraw text{.bounds = {{2, 2}, {20, 10}}, .scale = scale};
  text.glyphs.push_back(cgpui::TextGlyphPaint{
      .origin = {2, 2}, .device_origin = {3, 3}, .device_advance = 9});
  const cgpui::TextDraw device_text = cgpui::renderer_to_device_pixels(text, scale);
  if (!same(device_text.bounds, 3, 3, 30, 15) ||
      device_text.glyphs[0].device_origin.x != 3 ||
      device_text.glyphs[0].device_advance != 9) {
    return fail(5, "text bounds scale; glyph device fields are already device pixels");
  }
  const cgpui::SolidRect unit = cgpui::renderer_to_device_pixels(
      cgpui::SolidRect{.rect = {{1.25F, 2}, {3, 4}}}, cgpui::DpiScale{1.0F});
  if (!same(unit.rect, 1.25F, 2, 3, 4)) {
    return fail(6, "unit scale must leave draws unchanged");
  }
  return 0;
}
