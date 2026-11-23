#include "cgpui/renderer/glyph_cache.hpp"
#include "cgpui/renderer/planning/glyph_geometry.hpp"
#include "cgpui/ui/paint.hpp"
#include "renderer_pixel_test_support.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

// Guards the ink extent of a text run: no clip, scissor or quad size may cut
// ink right of the last pen position or left of a negative bearing ("blur",
// "f", "W", "j", italics). Draws through PaintList at scale 1 and 1.5 into a
// real swapchain and compares the rendered ink extent with the ink of the
// glyph bitmaps. Written for the reported clipped "r" in
// example_backdrop_blur, which turned out to be the black 6 px stripe at
// logical x 480..486 hiding the end of the black "r" (ink ends at x 484).
namespace {

const cgpui::FontDescriptor font{.family = "Segoe UI"};
const cgpui::FontDescriptor italic{.family = "Segoe UI",
                                   .style = cgpui::FontStyle::italic};

struct Extent {
  int left = 1 << 20;
  int right = -1;
};

cgpui::TextDraw draw_for(const std::string& content,
                         const cgpui::FontDescriptor& face, float scale) {
  cgpui::PaintList paints;
  paints.set_scale({scale});
  // The example's 820 x 60 rect; narrower rects wrap the full title.
  paints.fill_text({{16, 8}, {820, 60}}, cgpui::rgb(255, 255, 255), content,
                   face, 48.0F);
  const auto& text = paints.commands().front().text;
  return {.bounds = text.bounds, .color = text.color, .font = text.font,
          .content = text.content, .byte_length = text.byte_length,
          .font_size = text.font_size, .scale = text.scale,
          .device_font_size = text.device_font_size, .glyphs = text.glyphs,
          .lines = text.lines, .clip_rect = paints.commands().front().clip_rect};
}

// Ink of every glyph bitmap placed at its quad origin, in device pixels.
Extent bitmap_ink(const cgpui::TextDraw& draw) {
  cgpui::GlyphCache cache;
  const auto quads = cgpui::build_textured_glyph_quads(draw, cache);
  Extent extent;
  for (const auto& quad : quads) {
    for (const auto& upload : cache.upload_records()) {
      if (upload.page_index != quad.page_index ||
          upload.atlas_bounds.origin.x != quad.atlas_bounds.origin.x ||
          upload.atlas_bounds.origin.y != quad.atlas_bounds.origin.y) {
        continue;
      }
      const int origin =
          static_cast<int>(std::lround(quad.device_bounds.origin.x));
      for (std::uint32_t y = 0; y < upload.height; ++y) {
        for (std::uint32_t x = 0; x < upload.width; ++x) {
          if (upload.alpha[y * upload.stride + x] > 64) {
            extent.left = (std::min)(extent.left, origin + static_cast<int>(x));
            extent.right =
                (std::max)(extent.right, origin + static_cast<int>(x) + 1);
          }
        }
      }
    }
  }
  return extent;
}

Extent rendered_ink(const cgpui::RendererFramePixels& pixels) {
  Extent extent;
  for (std::uint32_t y = 0; y < pixels.height; ++y) {
    for (std::uint32_t x = 0; x < pixels.width; ++x) {
      const auto color = pixels.pixel_rgba8(x, y);
      if (color && (*color)[0] > 64) {
        extent.left = (std::min)(extent.left, static_cast<int>(x));
        extent.right = (std::max)(extent.right, static_cast<int>(x) + 1);
      }
    }
  }
  return extent;
}

int check(cgpui_test::RendererPixelTestWindow& window, const std::string& content,
          const cgpui::FontDescriptor& face, float scale) {
  const auto draw = draw_for(content, face, scale);
  const Extent expected = bitmap_ink(draw);
  auto pixels = cgpui_test::capture_pixels_at_scale(
      window, cgpui::DpiScale{scale}, {.a = 1.0F},
      [&](cgpui::RenderFrame& frame) { frame.draw_text(draw); });
  if (!pixels || !pixels->valid() || expected.right < 0) return 1;
  const Extent actual = rendered_ink(*pixels);
  std::printf("\"%s\"%s @%.1f: bitmap ink %d..%d, rendered ink %d..%d\n",
              content.c_str(), face.style == cgpui::FontStyle::italic ? " italic" : "",
              scale, expected.left, expected.right, actual.left, actual.right);
  // One device pixel of tolerance for coverage filtering at the ink edge.
  if (actual.right < expected.right - 1) return 2;
  if (actual.left > expected.left + 1) return 3;
  return 0;
}

} // namespace

int main() {
  cgpui_test::RendererPixelTestWindow window(
      L"CGPUIRendererTextInkExtentPixelTestWindow", L"CGPUI Text Ink Extent Test");
  (void)window.resize_client(1300, 140);
  for (const float scale : {1.0F, 1.5F}) {
    for (const char* content : {"blur", "f", "W", "j"}) {
      if (const int result = check(window, content, font, scale)) return result;
    }
    if (const int r = check(window, "CGPUI backdrop blur", {}, scale)) return 20 + r;
    for (const char* content : {"blur", "f", "j"}) {
      if (const int result = check(window, content, italic, scale)) {
        return 10 + result;
      }
    }
  }
  return 0;
}
