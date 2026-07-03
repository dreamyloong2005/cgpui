#pragma once

#include "cgpui/ui/text_shape.hpp"

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
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;

  friend bool operator==(const GlyphAtlasKey&, const GlyphAtlasKey&) = default;
};

struct TextGlyphPaint {
  GlyphAtlasKey key;
  Point origin;
  float advance = 0.0F;
  Point device_origin;
  float device_advance = 0.0F;
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
