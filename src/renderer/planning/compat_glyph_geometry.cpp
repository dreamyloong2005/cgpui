#include "cgpui/renderer/planning/glyph_geometry.hpp"

namespace cgpui {

void vulkan_consume_text_draw(const TextDraw& text, GlyphCache& glyph_cache) {
  consume_text_draw(text, glyph_cache);
}

std::vector<TexturedGlyphQuad> vulkan_build_textured_glyph_quads(
    const TextDraw& text,
    GlyphCache& glyph_cache) {
  return build_textured_glyph_quads(text, glyph_cache);
}

} // namespace cgpui
