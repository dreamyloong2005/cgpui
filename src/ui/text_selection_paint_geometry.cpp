#include "text_selection_paint_geometry.hpp"

#include <algorithm>

namespace cgpui {
namespace {

float text_line_x_for_byte_offset(
    const TextMeasurement& measurement,
    const TextWrapLine& line,
    std::size_t byte_offset) {
  const std::size_t offset =
      std::clamp(byte_offset, line.byte_start, line.byte_end);
  float x = line.origin.x;
  if (!measurement.grapheme_columns.empty()) {
    for (std::size_t index = line.column_start; index < line.column_end;
         ++index) {
      const TextGraphemeColumn& column = measurement.grapheme_columns[index];
      if (offset <= column.byte_start) {
        return x;
      }
      if (offset < column.byte_end) {
        return x;
      }
      x += column.advance;
    }
    return x;
  }

  const TextShapeRun& run = measurement.shape_run;
  const std::size_t glyph_end = std::min(line.glyph_end, run.glyphs.size());
  for (std::size_t index = line.glyph_start; index < glyph_end; ++index) {
    const TextGlyphRun& glyph = run.glyphs[index];
    if (offset <= glyph.byte_offset) {
      return x;
    }
    if (offset < glyph.byte_offset + glyph.byte_length) {
      return x;
    }
    x += glyph.advance;
  }
  return x;
}

TextWrapLayout text_selection_wrap_layout(
    std::string_view text,
    const FontDescriptor& font,
    float font_size,
    DpiScale scale,
    float max_width,
    TextMeasurement& measurement) {
  measurement = measure_text(text, font, font_size, scale);
  return wrap_text_measurement(measurement, max_width);
}

Rect text_line_rect_for_range(
    Rect bounds,
    const TextMeasurement& measurement,
    const TextWrapLine& line,
    std::size_t start,
    std::size_t end) {
  const float start_x = text_line_x_for_byte_offset(measurement, line, start);
  const float end_x = text_line_x_for_byte_offset(measurement, line, end);
  return Rect{
      .origin =
          {
              .x = bounds.origin.x + start_x,
              .y = bounds.origin.y + line.origin.y,
          },
      .size =
          {
              .width = std::max(0.0F, end_x - start_x),
              .height = line.metrics.line_height,
          },
  };
}

} // namespace

void paint_text_selection_ranges(
    PaintList& paint_list,
    Rect bounds,
    Color color,
    std::string_view text,
    const FontDescriptor& font,
    float font_size,
    DpiScale scale,
    TextSelectionRange selection) {
  if (selection.collapsed) {
    return;
  }

  TextMeasurement measurement;
  const TextWrapLayout wrap_layout = text_selection_wrap_layout(
      text,
      font,
      font_size,
      scale,
      bounds.size.width,
      measurement);
  for (const TextWrapLine& line : wrap_layout.lines) {
    const std::size_t start = std::max(selection.start, line.byte_start);
    const std::size_t end = std::min(selection.end, line.byte_end);
    if (start >= end) {
      continue;
    }
    paint_list.fill_text_selection(
        text_line_rect_for_range(bounds, measurement, line, start, end),
        color,
        TextSelectionRange{.start = start, .end = end, .collapsed = false},
        font_size);
  }
}

void paint_text_caret_geometry(
    PaintList& paint_list,
    Rect bounds,
    Color color,
    std::string_view text,
    const FontDescriptor& font,
    float font_size,
    DpiScale scale,
    std::size_t byte_offset) {
  paint_list.fill_text_caret(
      text_caret_rect(bounds, text, font, font_size, scale, byte_offset),
      color,
      byte_offset,
      font_size);
}

Rect text_caret_rect(
    Rect bounds,
    std::string_view text,
    const FontDescriptor& font,
    float font_size,
    DpiScale scale,
    std::size_t byte_offset) {
  TextMeasurement measurement;
  const TextWrapLayout wrap_layout = text_selection_wrap_layout(
      text,
      font,
      font_size,
      scale,
      bounds.size.width,
      measurement);
  const TextWrapLine* caret_line = wrap_layout.lines.empty()
      ? nullptr
      : &wrap_layout.lines.back();
  for (const TextWrapLine& line : wrap_layout.lines) {
    if (byte_offset >= line.byte_start && byte_offset <= line.byte_end) {
      caret_line = &line;
      break;
    }
  }

  const float x = caret_line == nullptr
      ? 0.0F
      : text_line_x_for_byte_offset(measurement, *caret_line, byte_offset);
  const float y = caret_line == nullptr ? 0.0F : caret_line->origin.y;
  const float height = caret_line == nullptr
      ? font_size
      : caret_line->metrics.line_height;
  return Rect{
      .origin = {.x = bounds.origin.x + x, .y = bounds.origin.y + y},
      .size = {.width = 1.0F, .height = height},
  };
}

} // namespace cgpui
