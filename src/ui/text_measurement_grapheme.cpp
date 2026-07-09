#include "text_measurement_internal.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace cgpui {
namespace {

struct DecodedMeasurementCodepoint {
  std::uint32_t value = 0;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
};

bool is_text_measurement_utf8_continuation(char value) {
  return (static_cast<unsigned char>(value) & 0xC0U) == 0x80U;
}

DecodedMeasurementCodepoint decode_measurement_codepoint(
    const TextShapeRun& run,
    std::size_t glyph_index) {
  if (glyph_index >= run.glyphs.size()) {
    return {};
  }
  const TextGlyphRun& glyph = run.glyphs[glyph_index];
  if (glyph.byte_offset >= run.text.size()) {
    return DecodedMeasurementCodepoint{
        .byte_start = run.text.size(),
        .byte_end = run.text.size(),
    };
  }

  std::size_t byte_end =
      std::min(glyph.byte_offset + glyph.byte_length, run.text.size());
  while (byte_end < run.text.size() &&
         is_text_measurement_utf8_continuation(run.text[byte_end])) {
    byte_end += 1;
  }

  const auto first = static_cast<unsigned char>(run.text[glyph.byte_offset]);
  if (first < 0x80U) {
    return DecodedMeasurementCodepoint{
        .value = first,
        .byte_start = glyph.byte_offset,
        .byte_end = glyph.byte_offset + 1,
    };
  }
  if (byte_end - glyph.byte_offset == 2) {
    const auto second =
        static_cast<unsigned char>(run.text[glyph.byte_offset + 1]);
    return DecodedMeasurementCodepoint{
        .value = ((static_cast<std::uint32_t>(first) & 0x1FU) << 6U) |
            (static_cast<std::uint32_t>(second) & 0x3FU),
        .byte_start = glyph.byte_offset,
        .byte_end = byte_end,
    };
  }
  if (byte_end - glyph.byte_offset == 3) {
    const auto second =
        static_cast<unsigned char>(run.text[glyph.byte_offset + 1]);
    const auto third =
        static_cast<unsigned char>(run.text[glyph.byte_offset + 2]);
    return DecodedMeasurementCodepoint{
        .value = ((static_cast<std::uint32_t>(first) & 0x0FU) << 12U) |
            ((static_cast<std::uint32_t>(second) & 0x3FU) << 6U) |
            (static_cast<std::uint32_t>(third) & 0x3FU),
        .byte_start = glyph.byte_offset,
        .byte_end = byte_end,
    };
  }
  if (byte_end - glyph.byte_offset == 4) {
    const auto second =
        static_cast<unsigned char>(run.text[glyph.byte_offset + 1]);
    const auto third =
        static_cast<unsigned char>(run.text[glyph.byte_offset + 2]);
    const auto fourth =
        static_cast<unsigned char>(run.text[glyph.byte_offset + 3]);
    return DecodedMeasurementCodepoint{
        .value = ((static_cast<std::uint32_t>(first) & 0x07U) << 18U) |
            ((static_cast<std::uint32_t>(second) & 0x3FU) << 12U) |
            ((static_cast<std::uint32_t>(third) & 0x3FU) << 6U) |
            (static_cast<std::uint32_t>(fourth) & 0x3FU),
        .byte_start = glyph.byte_offset,
        .byte_end = byte_end,
    };
  }
  return DecodedMeasurementCodepoint{
      .value = first,
      .byte_start = glyph.byte_offset,
      .byte_end = byte_end,
  };
}

bool is_text_measurement_combining_mark(std::uint32_t value) {
  return (value >= 0x0300U && value <= 0x036FU) ||
      (value >= 0x1AB0U && value <= 0x1AFFU) ||
      (value >= 0x1DC0U && value <= 0x1DFFU) ||
      (value >= 0x20D0U && value <= 0x20FFU) ||
      (value >= 0xFE20U && value <= 0xFE2FU);
}

bool is_text_measurement_variation_selector(std::uint32_t value) {
  return (value >= 0xFE00U && value <= 0xFE0FU) ||
      (value >= 0xE0100U && value <= 0xE01EFU);
}

bool is_text_measurement_regional_indicator(std::uint32_t value) {
  return value >= 0x1F1E6U && value <= 0x1F1FFU;
}

bool is_text_measurement_zero_width_joiner(std::uint32_t value) {
  return value == 0x200DU;
}

bool text_grapheme_column_includes_codepoint(
    std::uint32_t codepoint,
    bool follows_zero_width_joiner,
    std::size_t regional_indicator_count) {
  return is_text_measurement_combining_mark(codepoint) ||
      is_text_measurement_variation_selector(codepoint) ||
      follows_zero_width_joiner ||
      (regional_indicator_count == 1U &&
       is_text_measurement_regional_indicator(codepoint));
}

void append_text_grapheme_column_glyph(
    TextGraphemeColumn& column,
    const TextShapeRun& run,
    std::size_t glyph_index) {
  const TextGlyphRun& glyph = run.glyphs[glyph_index];
  column.glyph_end = glyph_index + 1U;
  column.byte_end =
      std::min(glyph.byte_offset + glyph.byte_length, run.text.size());
  column.advance += glyph.advance;
  column.device_advance += glyph.advance * normalized_scale(run.scale);
}

} // namespace

std::vector<TextGraphemeColumn> build_text_grapheme_columns(
    const TextShapeRun& run) {
  std::vector<TextGraphemeColumn> columns;
  columns.reserve(run.glyphs.size());

  std::size_t glyph_index = 0;
  while (glyph_index < run.glyphs.size()) {
    const DecodedMeasurementCodepoint first =
        decode_measurement_codepoint(run, glyph_index);
    TextGraphemeColumn column{
        .column_start = columns.size(),
        .column_end = columns.size() + 1U,
        .byte_start = first.byte_start,
        .byte_end = first.byte_end,
        .glyph_start = glyph_index,
        .glyph_end = glyph_index,
    };

    std::size_t regional_indicator_count =
        is_text_measurement_regional_indicator(first.value) ? 1U : 0U;
    bool follows_zero_width_joiner = false;
    append_text_grapheme_column_glyph(column, run, glyph_index);
    glyph_index += 1;

    while (glyph_index < run.glyphs.size()) {
      const DecodedMeasurementCodepoint current =
          decode_measurement_codepoint(run, glyph_index);
      if (is_text_measurement_zero_width_joiner(current.value)) {
        append_text_grapheme_column_glyph(column, run, glyph_index);
        glyph_index += 1;
        follows_zero_width_joiner = true;
        continue;
      }

      if (!text_grapheme_column_includes_codepoint(
              current.value,
              follows_zero_width_joiner,
              regional_indicator_count)) {
        break;
      }

      append_text_grapheme_column_glyph(column, run, glyph_index);
      glyph_index += 1;
      follows_zero_width_joiner = false;
      if (is_text_measurement_regional_indicator(current.value)) {
        regional_indicator_count += 1U;
      }
    }

    columns.push_back(column);
  }
  return columns;
}

} // namespace cgpui
