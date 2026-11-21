#include "renderer_text_glyph_placement_internal.hpp"

#include <cmath>

namespace cgpui {

// Natively shaped glyphs rasterize by glyph id + face + size + one of four
// horizontal subpixel buckets; the pen snaps to whole pixels and the
// baseline to a whole row so coverage is not resampled.
GlyphPlacement native_glyph_placement(const TextGlyphPaint& glyph) {
  GlyphPlacement placement{.raster = glyph};
  float pen_x = std::floor(glyph.device_origin.x);
  int bucket = static_cast<int>(
      std::lround((glyph.device_origin.x - pen_x) * 4.0F));
  if (bucket >= 4) {
    pen_x += 1.0F;
    bucket = 0;
  }
  placement.device_pen = Point{
      .x = pen_x,
      .y = std::round(glyph.device_origin.y + glyph.device_baseline),
  };
  GlyphAtlasKey& key = placement.raster.key;
  key.text.clear();
  key.glyph_index = 0;
  key.byte_offset = 0;
  key.byte_length = 0;
  key.raster_advance = 0.0F;
  key.subpixel_x = static_cast<std::uint8_t>(bucket);
  return placement;
}

GlyphPlacement cell_glyph_placement(
    const TextGlyphPaint& glyph,
    const std::string& content) {
  GlyphPlacement placement{.raster = glyph, .device_pen = glyph.device_origin};
  TextGlyphPaint& raster_glyph = placement.raster;
  raster_glyph.key.raster_advance = glyph.device_advance;
  if (glyph.key.byte_offset < content.size() &&
      glyph.key.byte_length <= content.size() - glyph.key.byte_offset) {
    raster_glyph.key.text =
        content.substr(glyph.key.byte_offset, glyph.key.byte_length);
  }
  // Reuse a real cluster at any position. Offsets stay on the draw metadata;
  // they must not multiply atlas allocations or alias a different string.
  if (!raster_glyph.key.text.empty()) {
    raster_glyph.key.glyph_index = 0;
    if (raster_glyph.key.shaping_backend ==
        TextShapingBackend::deterministic_fallback) {
      raster_glyph.key.glyph_id = 0;
    }
    raster_glyph.key.byte_offset = 0;
    raster_glyph.key.byte_length = raster_glyph.key.text.size();
  }
  return placement;
}

} // namespace cgpui
