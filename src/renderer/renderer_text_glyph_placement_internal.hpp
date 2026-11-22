#pragma once

#include "cgpui/renderer/text/text_glyphs.hpp"

#include <string>

namespace cgpui {

// Atlas identity for one painted glyph and the device pen its rasterized
// bearings are relative to.
struct GlyphPlacement {
  TextGlyphPaint raster;
  Point device_pen;
};

// Natively shaped glyph: glyph id + face + size + subpixel bucket key, pen
// snapped to a whole pixel, baseline snapped to a whole row.
[[nodiscard]] GlyphPlacement native_glyph_placement(const TextGlyphPaint& glyph);

// Cell-fitted deterministic glyph keyed by its source cluster text.
[[nodiscard]] GlyphPlacement cell_glyph_placement(
    const TextGlyphPaint& glyph,
    const std::string& content);

} // namespace cgpui
