#include "text_measurement_internal.hpp"

#include <algorithm>
#include <cstdint>

namespace cgpui {
namespace {

std::uint32_t decode_text_bidi_codepoint(
    const TextShapeRun& run,
    const TextGraphemeColumn& column) {
  if (column.byte_start >= run.text.size()) {
    return 0;
  }
  const auto byte_at = [&run](std::size_t index) {
    return static_cast<unsigned char>(run.text[index]);
  };

  const std::size_t byte_end =
      std::min(column.byte_end, run.text.size());
  const unsigned char first = byte_at(column.byte_start);
  if ((first & 0x80U) == 0U || byte_end - column.byte_start == 1U) {
    return first;
  }
  if (byte_end - column.byte_start == 2U) {
    return ((static_cast<std::uint32_t>(first) & 0x1FU) << 6U) |
        (static_cast<std::uint32_t>(byte_at(column.byte_start + 1U)) & 0x3FU);
  }
  if (byte_end - column.byte_start == 3U) {
    return ((static_cast<std::uint32_t>(first) & 0x0FU) << 12U) |
        ((static_cast<std::uint32_t>(byte_at(column.byte_start + 1U)) & 0x3FU)
         << 6U) |
        (static_cast<std::uint32_t>(byte_at(column.byte_start + 2U)) & 0x3FU);
  }
  if (byte_end - column.byte_start >= 4U) {
    return ((static_cast<std::uint32_t>(first) & 0x07U) << 18U) |
        ((static_cast<std::uint32_t>(byte_at(column.byte_start + 1U)) & 0x3FU)
         << 12U) |
        ((static_cast<std::uint32_t>(byte_at(column.byte_start + 2U)) & 0x3FU)
         << 6U) |
        (static_cast<std::uint32_t>(byte_at(column.byte_start + 3U)) & 0x3FU);
  }
  return first;
}

TextShapingDirection text_bidi_base_direction(const TextShapeRun& run) {
  return run.direction == TextShapingDirection::right_to_left
      ? TextShapingDirection::right_to_left
      : TextShapingDirection::left_to_right;
}

TextShapingDirection classify_text_bidi_direction(
    std::uint32_t codepoint,
    TextShapingDirection base_direction) {
  if ((0x0590U <= codepoint && codepoint <= 0x08FFU) ||
      (0xFB1DU <= codepoint && codepoint <= 0xFDFFU) ||
      (0xFE70U <= codepoint && codepoint <= 0xFEFFU)) {
    return TextShapingDirection::right_to_left;
  }
  if (codepoint == 0 || codepoint == '\n' || codepoint == '\r' ||
      codepoint == '\t' || codepoint == ' ') {
    return base_direction;
  }
  return TextShapingDirection::left_to_right;
}

std::uint8_t text_bidi_embedding_level(TextShapingDirection direction) {
  return direction == TextShapingDirection::right_to_left ? 1U : 0U;
}

void append_text_bidi_run_span(
    std::vector<TextBidiRun>& runs,
    TextShapingDirection direction,
    std::uint8_t embedding_level,
    const TextGraphemeColumn& column) {
  if (runs.empty() || runs.back().direction != direction ||
      runs.back().embedding_level != embedding_level) {
    runs.push_back(TextBidiRun{
        .direction = direction,
        .embedding_level = embedding_level,
        .column_start = column.column_start,
        .column_end = column.column_start,
        .byte_start = column.byte_start,
        .byte_end = column.byte_start,
        .glyph_start = column.glyph_start,
        .glyph_end = column.glyph_start,
    });
  }

  TextBidiRun& run = runs.back();
  run.column_end = column.column_end;
  run.byte_end = column.byte_end;
  run.glyph_end = column.glyph_end;
  run.advance += column.advance;
  run.device_advance += column.device_advance;
}

} // namespace

std::vector<TextBidiRun> build_text_bidi_runs(
    const TextShapeRun& run,
    std::span<const TextGraphemeColumn> columns) {
  std::vector<TextBidiRun> bidi_runs;
  bidi_runs.reserve(columns.size());
  const TextShapingDirection base_direction = text_bidi_base_direction(run);
  for (const TextGraphemeColumn& column : columns) {
    const std::uint32_t codepoint =
        decode_text_bidi_codepoint(run, column);
    const TextShapingDirection direction =
        classify_text_bidi_direction(codepoint, base_direction);
    append_text_bidi_run_span(
        bidi_runs,
        direction,
        text_bidi_embedding_level(direction),
        column);
  }
  return bidi_runs;
}

} // namespace cgpui
