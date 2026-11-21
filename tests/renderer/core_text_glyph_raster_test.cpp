#include "cgpui/ui/text.hpp"
#include "cgpui/ui/text_glyph_rasterizer.hpp"

#include <algorithm>
#include <cstdio>
#include <string>

// Core Text glyph-id rasterization: bitmaps hold real ink with bearings
// relative to the pen on the baseline, the subpixel bucket shifts the ink,
// and a space has no ink.
namespace {

int fail(int code, const char* message) {
  std::fprintf(stderr, "core text raster: %s\n", message);
  return code;
}

std::size_t ink(const cgpui::RasterizedGlyph& glyph) {
  return static_cast<std::size_t>(std::ranges::count_if(
      glyph.bitmap.alpha, [](std::uint8_t value) { return value > 128; }));
}

// Mean x of the coverage, in pixels from the pen.
double centroid_x(const cgpui::RasterizedGlyph& glyph) {
  double sum = 0.0;
  double weight = 0.0;
  for (std::uint32_t y = 0; y < glyph.bitmap.height; ++y) {
    for (std::uint32_t x = 0; x < glyph.bitmap.width; ++x) {
      const double value = glyph.bitmap.alpha[y * glyph.bitmap.stride + x];
      sum += value * (x + 0.5 + glyph.left_bearing);
      weight += value;
    }
  }
  return weight > 0.0 ? sum / weight : 0.0;
}

}  // namespace

int main() {
  const std::string text = "Hg ";
  const cgpui::TextShapeRun run =
      cgpui::shape_text(text, cgpui::FontDescriptor{.family = "Helvetica Neue"}, 48.0F);
  if (run.used_backend != cgpui::TextShapingBackend::core_text || run.glyphs.size() != 3) {
    return fail(1, "Helvetica Neue did not shape with Core Text");
  }
  const auto paints = cgpui::text_glyph_paint_metadata(run, cgpui::Point{});
  if (paints.size() != 3) return fail(2, "paint metadata size");
  auto raster = [&](std::size_t index, std::uint8_t bucket) {
    cgpui::TextGlyphPaint glyph = paints[index];
    glyph.key.subpixel_x = bucket;
    return cgpui::rasterize_text_glyph(glyph, "");
  };
  const auto h = raster(0, 0);
  const auto g = raster(1, 0);
  const auto space = raster(2, 0);
  if (!h || !g || !space) return fail(3, "rasterization failed");
  // H sits on the baseline: its ink ends at about row 0; g descends below.
  const float h_bottom = h->top_bearing + static_cast<float>(h->bitmap.height);
  const float g_bottom = g->top_bearing + static_cast<float>(g->bitmap.height);
  if (ink(*h) < 200 || h->top_bearing > -30.0F || h_bottom < -1.0F || h_bottom > 3.0F ||
      g_bottom < 6.0F || h->left_bearing < -2.0F || h->left_bearing > 8.0F) {
    std::fprintf(stderr, "H %ux%u bearings %.1f %.1f, g bottom %.1f\n", h->bitmap.width,
                 h->bitmap.height, h->left_bearing, h->top_bearing, g_bottom);
    return fail(4, "glyph bearings are not relative to the baseline pen");
  }
  if (ink(*space) != 0) return fail(5, "a space must have no ink");
  // Two quarter-pixel buckets shift the ink right by about half a pixel.
  const auto shifted = raster(0, 2);
  if (!shifted) return fail(6, "shifted rasterization failed");
  const double shift = centroid_x(*shifted) - centroid_x(*h);
  if (shift < 0.35 || shift > 0.65) {
    std::fprintf(stderr, "centroid shift %.3f\n", shift);
    return fail(7, "subpixel bucket did not shift the ink");
  }
  return 0;
}
