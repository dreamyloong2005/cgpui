#pragma once

#include "cgpui/ui/text_glyphs.hpp"

#include <cstddef>

namespace cgpui {

// Paint record for run.glyphs[index] whose line top-left is `line_origin`
// and pen x is `pen_x`. Natively shaped glyphs carry their face identity
// and the line baseline; deterministic glyphs keep the fitted-cell form.
[[nodiscard]] TextGlyphPaint text_glyph_paint_for(
    const TextShapeRun& run,
    std::size_t index,
    float pen_x,
    Point line_origin,
    float baseline);

} // namespace cgpui
