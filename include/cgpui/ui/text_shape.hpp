#pragma once

#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/text_font.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

struct TextGlyphRun {
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;
  float advance = 0.0F;
};

struct TextShapeRun {
  std::string text;
  FontDescriptor font;
  float font_size = 16.0F;
  DpiScale scale;
  std::vector<TextGlyphRun> glyphs;
  std::size_t byte_length = 0;
  float total_advance = 0.0F;
  float line_height = 16.0F;
  float device_font_size = 16.0F;
  float device_total_advance = 0.0F;
  float device_line_height = 16.0F;

  [[nodiscard]] std::size_t glyph_count() const;
};

[[nodiscard]] bool is_utf8_continuation_byte(char value);

[[nodiscard]] TextShapeRun shape_text(
    std::string_view text,
    FontDescriptor font = {},
    float font_size = 16.0F,
    DpiScale scale = {});

} // namespace cgpui
