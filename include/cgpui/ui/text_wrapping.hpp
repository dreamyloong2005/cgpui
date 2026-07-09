#pragma once

#include "cgpui/ui/text_glyphs.hpp"
#include "cgpui/ui/text_measurement.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace cgpui {

struct TextWrapLine {
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  std::size_t glyph_start = 0;
  std::size_t glyph_end = 0;
  std::size_t column_start = 0;
  std::size_t column_end = 0;
  Point origin;
  Size size;
};

struct TextWrapLayout {
  std::vector<TextWrapLine> lines;
  Size logical_size;
  Size device_size;
  float max_width = 0.0F;
};

[[nodiscard]] TextWrapLine text_wrap_line_for_range(
    const TextShapeRun& run,
    std::size_t glyph_start,
    std::size_t glyph_end,
    float y,
    float width);

[[nodiscard]] TextWrapLine text_wrap_line_for_column_range(
    const TextMeasurement& measurement,
    std::size_t column_start,
    std::size_t column_end,
    float y,
    float width);

[[nodiscard]] TextWrapLayout wrap_text_measurement(
    const TextMeasurement& measurement,
    float max_width);

[[nodiscard]] std::vector<TextGlyphPaint> text_glyph_paint_metadata(
    const TextShapeRun& run,
    std::span<const TextWrapLine> lines,
    Point origin = {});

} // namespace cgpui
