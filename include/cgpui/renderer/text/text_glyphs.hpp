#pragma once

#include "cgpui/renderer/text/text_shape.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace cgpui {

struct GlyphAtlasKey {
  std::string font_family;
  float font_size = 16.0F;
  float scale = 1.0F;
  float device_font_size = 16.0F;
  std::size_t glyph_index = 0;
  std::uint32_t glyph_id = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;
  // The fallback shaper's numeric glyph_id is a position, not a font glyph.
  // Keep the actual cluster so different strings cannot alias in the atlas.
  std::string text;
  // Native coverage currently fits the supplied layout cell.
  float raster_advance = 0.0F;
  TextShapingBackend shaping_backend = TextShapingBackend::deterministic_fallback;
  // Native glyphs rasterize by glyph_id from this face; `text` is cleared.
  std::string font_face_path;
  std::uint32_t font_face_index = 0;
  bool synthetic_bold = false;
  bool synthetic_oblique = false;
  // Horizontal subpixel position bucket (quarters of a device pixel).
  std::uint8_t subpixel_x = 0;

  friend bool operator==(const GlyphAtlasKey&, const GlyphAtlasKey&) = default;
};

struct TextGlyphPaint {
  GlyphAtlasKey key;
  Point origin;
  float advance = 0.0F;
  Point device_origin;
  float device_advance = 0.0F;
  // Baseline below device_origin for natively shaped glyphs. Their
  // rasterized bearings are relative to (device_origin.x,
  // device_origin.y + device_baseline); zero for cell-fitted glyphs.
  float device_baseline = 0.0F;
};

struct GlyphBitmap {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t stride = 0;
  std::vector<std::uint8_t> alpha;

  [[nodiscard]] bool empty() const;

  [[nodiscard]] std::size_t byte_size() const;

  [[nodiscard]] std::uint8_t pixel(std::uint32_t x, std::uint32_t y) const;
};

struct GlyphRasterizerOptions {
  std::uint8_t foreground_alpha = 255;
  std::uint8_t background_alpha = 0;
  std::uint32_t padding = 1;
};

struct RasterizedGlyph {
  GlyphAtlasKey key;
  GlyphBitmap bitmap;
  float advance = 0.0F;
  float device_font_size = 16.0F;
  float left_bearing = 0.0F;
  float top_bearing = 0.0F;
  float baseline = 0.0F;
};

[[nodiscard]] std::uint32_t rasterized_glyph_dimension(float value);

[[nodiscard]] RasterizedGlyph rasterize_fallback_glyph(
    const TextGlyphPaint& glyph,
    GlyphRasterizerOptions options = {});

[[nodiscard]] std::vector<TextGlyphPaint> text_glyph_paint_metadata(
    const TextShapeRun& run,
    Point origin = {});

} // namespace cgpui
