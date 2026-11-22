#pragma once

#include "cgpui/renderer/text/text_glyph_rasterizer.hpp"

namespace cgpui {

[[nodiscard]] Result<RasterizedGlyph> rasterize_win32_text_glyph(
    const TextGlyphPaint& glyph,
    std::string_view utf8);

} // namespace cgpui
