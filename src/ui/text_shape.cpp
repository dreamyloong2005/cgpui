#include "cgpui/ui/text_shape.hpp"

#include <string>
#include <utility>

namespace cgpui {

std::size_t TextShapeRun::glyph_count() const {
  return glyphs.size();
}

bool is_utf8_continuation_byte(char value) {
  return (static_cast<unsigned char>(value) & 0xC0U) == 0x80U;
}

TextShapeRun shape_text(
    std::string_view text,
    FontDescriptor font,
    float font_size,
    DpiScale scale) {
  const float scale_value = normalized_scale(scale);
  TextShapeRun run{
      .text = std::string(text),
      .font = std::move(font),
      .font_size = font_size,
      .scale = scale,
      .byte_length = text.size(),
      .line_height = font_size,
      .device_font_size = font_size * scale_value,
      .device_line_height = font_size * scale_value,
  };
  const float fallback_advance = font_size * 0.5F;
  std::size_t byte_offset = 0;
  while (byte_offset < text.size()) {
    std::size_t byte_length = 1;
    while (byte_offset + byte_length < text.size() &&
           is_utf8_continuation_byte(text[byte_offset + byte_length])) {
      byte_length += 1;
    }
    run.glyphs.push_back(TextGlyphRun{
        .byte_offset = byte_offset,
        .byte_length = byte_length,
        .advance = fallback_advance,
    });
    run.total_advance += fallback_advance;
    run.device_total_advance += fallback_advance * scale_value;
    byte_offset += byte_length;
  }
  return run;
}

} // namespace cgpui
