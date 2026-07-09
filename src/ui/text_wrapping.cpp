#include "cgpui/ui/text_wrapping.hpp"

#include <algorithm>

namespace cgpui {
namespace {

bool text_wrap_column_is_hard_break(
    const TextMeasurement& measurement,
    std::size_t column_index) {
  if (column_index >= measurement.grapheme_columns.size()) {
    return false;
  }
  const TextGraphemeColumn& column =
      measurement.grapheme_columns[column_index];
  const std::string& text = measurement.shape_run.text;
  return column.byte_start < column.byte_end &&
      column.byte_end <= text.size() &&
      text[column.byte_start] == '\n';
}

void text_wrap_line_assign_bidi_runs(
    TextWrapLine& line,
    const TextMeasurement& measurement) {
  line.bidi_run_start = measurement.bidi_runs.size();
  line.bidi_run_end = measurement.bidi_runs.size();
  for (std::size_t index = 0; index < measurement.bidi_runs.size(); ++index) {
    const TextBidiRun& bidi_run = measurement.bidi_runs[index];
    if (bidi_run.column_end <= line.column_start ||
        line.column_end <= bidi_run.column_start) {
      continue;
    }
    if (line.bidi_run_start == measurement.bidi_runs.size()) {
      line.bidi_run_start = index;
    }
    line.bidi_run_end = index + 1U;
  }
}

TextWrapLine text_wrap_line_for_range(
    const TextShapeRun& run,
    const TextLineMetrics& metrics,
    std::size_t glyph_start,
    std::size_t glyph_end,
    float y,
    float width) {
  TextWrapLine line{
      .glyph_start = glyph_start,
      .glyph_end = glyph_end,
      .column_start = glyph_start,
      .column_end = glyph_end,
      .metrics = metrics,
      .origin = {.x = 0.0F, .y = y},
      .size = {.width = width, .height = metrics.line_height},
  };
  if (glyph_start < glyph_end && glyph_end <= run.glyphs.size()) {
    line.byte_start = run.glyphs[glyph_start].byte_offset;
    const TextGlyphRun& last = run.glyphs[glyph_end - 1U];
    line.byte_end = last.byte_offset + last.byte_length;
  } else {
    line.byte_start = run.byte_length;
    line.byte_end = run.byte_length;
  }
  return line;
}

} // namespace

TextWrapLine text_wrap_line_for_range(
    const TextShapeRun& run,
    std::size_t glyph_start,
    std::size_t glyph_end,
    float y,
    float width) {
  return text_wrap_line_for_range(
      run,
      text_line_metrics_for_shape_run(run),
      glyph_start,
      glyph_end,
      y,
      width);
}

TextWrapLine text_wrap_line_for_column_range(
    const TextMeasurement& measurement,
    std::size_t column_start,
    std::size_t column_end,
    float y,
    float width) {
  const TextShapeRun& run = measurement.shape_run;
  const TextLineMetrics metrics = measurement.line_metrics;
  TextWrapLine line{
      .column_start = column_start,
      .column_end = column_end,
      .metrics = metrics,
      .origin = {.x = 0.0F, .y = y},
      .size = {.width = width, .height = metrics.line_height},
  };
  if (column_start == column_end &&
      column_start <= measurement.grapheme_columns.size()) {
    if (column_start < measurement.grapheme_columns.size()) {
      const TextGraphemeColumn& column =
          measurement.grapheme_columns[column_start];
      line.byte_start = column.byte_start;
      line.byte_end = column.byte_start;
      line.glyph_start = column.glyph_start;
      line.glyph_end = column.glyph_start;
    } else {
      line.byte_start = run.byte_length;
      line.byte_end = run.byte_length;
      line.glyph_start = run.glyphs.size();
      line.glyph_end = run.glyphs.size();
    }
  } else if (column_start < column_end &&
             column_end <= measurement.grapheme_columns.size()) {
    const TextGraphemeColumn& first =
        measurement.grapheme_columns[column_start];
    const TextGraphemeColumn& last =
        measurement.grapheme_columns[column_end - 1U];
    line.byte_start = first.byte_start;
    line.byte_end = last.byte_end;
    line.glyph_start = first.glyph_start;
    line.glyph_end = last.glyph_end;
  } else {
    line.byte_start = run.byte_length;
    line.byte_end = run.byte_length;
    line.glyph_start = run.glyphs.size();
    line.glyph_end = run.glyphs.size();
    line.column_start = measurement.grapheme_columns.size();
    line.column_end = measurement.grapheme_columns.size();
  }
  text_wrap_line_assign_bidi_runs(line, measurement);
  return line;
}

TextWrapLayout wrap_text_measurement(
    const TextMeasurement& measurement,
    float max_width) {
  const TextShapeRun& run = measurement.shape_run;
  const TextLineMetrics line_metrics = measurement.line_metrics;
  TextWrapLayout layout{
      .max_width = max_width,
      .base_direction = measurement.base_direction,
  };

  if (!measurement.grapheme_columns.empty()) {
    const auto append_column_line =
        [&](std::size_t column_start,
            std::size_t column_end,
            float y,
            float width,
            TextWrapBreakKind break_kind = TextWrapBreakKind::none) {
          TextWrapLine line = text_wrap_line_for_column_range(
              measurement,
              column_start,
              column_end,
              y,
              width);
          line.break_kind = break_kind;
          layout.lines.push_back(line);
          layout.logical_size.width =
              std::max(layout.logical_size.width, width);
        };

    std::size_t line_start = 0;
    float line_width = 0.0F;
    float y = 0.0F;
    for (std::size_t index = 0; index < measurement.grapheme_columns.size();
         ++index) {
      if (text_wrap_column_is_hard_break(measurement, index)) {
        append_column_line(
            line_start,
            index,
            y,
            line_width,
            TextWrapBreakKind::hard);
        y += line_metrics.line_height;
        line_start = index + 1U;
        line_width = 0.0F;
        continue;
      }
      const float advance = measurement.grapheme_columns[index].advance;
      if (max_width > 0.0F && index > line_start &&
          line_width + advance > max_width) {
        append_column_line(
            line_start,
            index,
            y,
            line_width,
            TextWrapBreakKind::soft);
        y += line_metrics.line_height;
        line_start = index;
        line_width = 0.0F;
      }
      line_width += advance;
    }

    append_column_line(
        line_start,
        measurement.grapheme_columns.size(),
        y,
        line_width);
    layout.logical_size.height = layout.lines.empty()
        ? 0.0F
        : layout.lines.back().origin.y + line_metrics.line_height;
    layout.device_size = to_device_pixels(layout.logical_size, run.scale);
    return layout;
  }

  const TextLineMetrics fallback_line_metrics =
      text_line_metrics_for_shape_run(run);
  const auto append_line =
      [&](std::size_t glyph_start,
          std::size_t glyph_end,
          float y,
          float width,
          TextWrapBreakKind break_kind = TextWrapBreakKind::none) {
        TextWrapLine line = text_wrap_line_for_range(
            run,
            fallback_line_metrics,
            glyph_start,
            glyph_end,
            y,
            width);
        line.break_kind = break_kind;
        layout.lines.push_back(line);
        layout.logical_size.width = std::max(layout.logical_size.width, width);
      };

  if (run.glyphs.empty() || max_width <= 0.0F ||
      run.total_advance <= max_width) {
    append_line(0, run.glyphs.size(), 0.0F, run.total_advance);
    layout.logical_size.height = fallback_line_metrics.line_height;
    layout.device_size = to_device_pixels(layout.logical_size, run.scale);
    return layout;
  }

  std::size_t line_start = 0;
  float line_width = 0.0F;
  float y = 0.0F;
  for (std::size_t index = 0; index < run.glyphs.size(); ++index) {
    const float advance = run.glyphs[index].advance;
    if (index > line_start && line_width + advance > max_width) {
      append_line(
          line_start,
          index,
          y,
          line_width,
          TextWrapBreakKind::soft);
      y += fallback_line_metrics.line_height;
      line_start = index;
      line_width = 0.0F;
    }
    line_width += advance;
  }

  append_line(line_start, run.glyphs.size(), y, line_width);
  layout.logical_size.height =
      layout.lines.empty()
          ? 0.0F
          : layout.lines.back().origin.y + fallback_line_metrics.line_height;
  layout.device_size = to_device_pixels(layout.logical_size, run.scale);
  return layout;
}

std::vector<TextGlyphPaint> text_glyph_paint_metadata(
    const TextShapeRun& run,
    std::span<const TextWrapLine> lines,
    Point origin) {
  std::vector<TextGlyphPaint> glyphs;
  glyphs.reserve(run.glyphs.size());
  for (const TextWrapLine& line : lines) {
    float x = origin.x + line.origin.x;
    const float y = origin.y + line.origin.y;
    const std::size_t glyph_end = std::min(line.glyph_end, run.glyphs.size());
    for (std::size_t index = line.glyph_start; index < glyph_end; ++index) {
      const TextGlyphRun& glyph = run.glyphs[index];
      const Point logical_origin{
          .x = x + glyph.offset.x,
          .y = y + glyph.offset.y,
      };
      glyphs.push_back(TextGlyphPaint{
          .key =
              GlyphAtlasKey{
                  .font_family = run.font.family,
                  .font_size = run.font_size,
                  .scale = normalized_scale(run.scale),
                  .device_font_size = run.device_font_size,
                  .glyph_index = index,
                  .glyph_id = glyph.glyph_id,
                  .byte_offset = glyph.byte_offset,
                  .byte_length = glyph.byte_length,
              },
          .origin = logical_origin,
          .advance = glyph.advance,
          .device_origin = to_device_pixels(logical_origin, run.scale),
          .device_advance = glyph.advance * normalized_scale(run.scale),
      });
      x += glyph.advance;
    }
  }
  return glyphs;
}

} // namespace cgpui
