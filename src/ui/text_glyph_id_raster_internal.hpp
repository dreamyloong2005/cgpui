#pragma once

#include "cgpui/ui/text_glyph_rasterizer.hpp"

namespace cgpui {

// Rasterizes key.glyph_id from the face named by key.font_face_path and
// key.font_face_index at key.device_font_size, shifted right by
// key.subpixel_x quarter pixels. Bearings are relative to the pen position
// on the baseline (top_bearing is negative above it); baseline is zero.
// Implemented by DirectWrite on Windows and FreeType on Linux.
[[nodiscard]] Result<RasterizedGlyph> rasterize_native_glyph_id(
    const TextGlyphPaint& glyph);

} // namespace cgpui
