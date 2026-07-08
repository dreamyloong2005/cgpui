#include "text_shaping_internal.hpp"

#include <string>
#include <utility>

namespace cgpui {

TextShapeRun shape_text_with_deterministic_fallback(
    TextShapingRequest request) {
  const float scale_value = normalized_scale(request.scale);
  TextShapeRun run{
      .text = std::string(request.text),
      .font = std::move(request.font),
      .font_size = request.font_size,
      .scale = request.scale,
      .byte_length = request.text.size(),
      .line_height = request.font_size,
      .device_font_size = request.font_size * scale_value,
      .device_line_height = request.font_size * scale_value,
      .requested_backend = request.backend.requested,
      .used_backend = request.backend.used,
      .fallback_reason = request.backend.fallback_reason,
      .direction = request.direction,
      .script = request.script,
      .language = std::move(request.language),
  };

  const float fallback_advance = request.font_size * 0.5F;
  std::size_t byte_offset = 0;
  std::uint32_t glyph_id = 0;
  while (byte_offset < request.text.size()) {
    std::size_t byte_length = 1;
    while (byte_offset + byte_length < request.text.size() &&
           is_utf8_continuation_byte(request.text[byte_offset + byte_length])) {
      byte_length += 1;
    }
    run.glyphs.push_back(TextGlyphRun{
        .glyph_id = glyph_id,
        .byte_offset = byte_offset,
        .byte_length = byte_length,
        .advance = fallback_advance,
    });
    run.total_advance += fallback_advance;
    run.device_total_advance += fallback_advance * scale_value;
    byte_offset += byte_length;
    glyph_id += 1;
  }
  return run;
}

} // namespace cgpui
