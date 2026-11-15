#pragma once

#include "cgpui/ui/text_glyph_rasterizer.hpp"

namespace cgpui {

[[nodiscard]] Result<RasterizedGlyph> rasterize_win32_text_glyph(
    const TextGlyphPaint& glyph,
    std::string_view utf8);

} // namespace cgpui
