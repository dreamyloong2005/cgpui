#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/renderer/text/text_glyphs.hpp"

#include <string_view>

namespace cgpui {

// Windows uses DirectWrite and platform font fallback. Other platforms retain
// the deterministic placeholder rasterizer until a native backend is added.
// Native coverage currently accepts deterministic-fallback source clusters.
// UTF-8 supplies the character, not the fallback shaper's positional glyph_id.
// HarfBuzz glyph runs require their own native face/cluster mapping and are
// rejected on Windows until that integration exists. Coverage fits the supplied
// advance/line cell; native shaping and interaction metrics remain separate.
//
// Bitmap dimensions, advance, bearings and baseline are device pixels.
// Bearings locate the tight bitmap's top-left relative to the glyph cell's
// top-left; baseline is measured downwards from that same cell origin.
// Whitespace ink is represented by a transparent 1x1 bitmap. An empty source
// preserves the transparent cell dimensions for metadata-only geometry users.
[[nodiscard]] Result<RasterizedGlyph> rasterize_text_glyph(
    const TextGlyphPaint& glyph,
    std::string_view utf8);

} // namespace cgpui
