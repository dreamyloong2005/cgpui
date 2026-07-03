#include "vulkan_report_internal.hpp"

namespace cgpui {

void vulkan_consume_text_draw(const TextDraw& text, GlyphCache& glyph_cache) {
  (void)vulkan_build_textured_glyph_quads(text, glyph_cache);
}

} // namespace cgpui
