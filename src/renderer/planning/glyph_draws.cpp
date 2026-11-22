#include "report_internal.hpp"

namespace cgpui {

void consume_text_draw(const TextDraw& text, GlyphCache& glyph_cache) {
  (void)build_textured_glyph_quads(text, glyph_cache);
}

} // namespace cgpui
