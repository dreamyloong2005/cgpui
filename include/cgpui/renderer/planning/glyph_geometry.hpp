#pragma once

#include "cgpui/renderer/glyph_cache.hpp"

namespace cgpui {

void consume_text_draw(const TextDraw& text, GlyphCache& glyph_cache);
std::vector<TexturedGlyphQuad> build_textured_glyph_quads(
    const TextDraw& text,
    GlyphCache& glyph_cache);

} // namespace cgpui
