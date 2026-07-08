#include "text_shaping_internal.hpp"

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
    DpiScale scale,
    TextShapingOptions options) {
  return shape_text_with_selected_backend(TextShapingRequest{
      .text = text,
      .font = std::move(font),
      .font_size = font_size,
      .scale = scale,
      .backend = select_text_shaping_backend(options),
  });
}

} // namespace cgpui
