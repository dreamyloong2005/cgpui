#include "text_measurement_internal.hpp"

#include "text_shaping_native_internal.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace cgpui {
namespace {

constexpr std::size_t unset_glyph = std::numeric_limits<std::size_t>::max();

struct ShapedCluster {
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  std::size_t glyph_start = 0;
  std::size_t glyph_end = 0;
  float advance = 0.0F;
};

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

bool text_measurement_is_crlf_pair(char32_t previous, char32_t current) {
  return previous == U'\r' && current == U'\n';
}

bool text_grapheme_column_includes_codepoint(
    std::uint32_t codepoint,
    bool follows_zero_width_joiner,
    std::size_t regional_indicator_count) {
  return is_text_measurement_combining_mark(codepoint) ||
      is_text_measurement_variation_selector(codepoint) ||
      is_text_measurement_zero_width_joiner(codepoint) ||
      follows_zero_width_joiner ||
      (regional_indicator_count == 1U &&
       is_text_measurement_regional_indicator(codepoint));
}

// Extended grapheme clusters for the subset CGPUI recognizes: CRLF,
// combining marks, variation selectors, ZWJ sequences, flag pairs.
std::vector<TextGraphemeColumn> segment_graphemes(const std::string& text) {
  std::vector<TextGraphemeColumn> columns;
  columns.reserve(text.size());
  std::size_t offset = 0;
  char32_t previous = U'\0';
  bool follows_joiner = false;
  std::size_t regional_count = 0;
  while (offset < text.size()) {
    std::size_t length = 0;
    const char32_t codepoint = text_shaping_decode_utf8(text, offset, &length);
    const bool extends = !columns.empty() &&
        (text_measurement_is_crlf_pair(previous, codepoint) ||
         text_grapheme_column_includes_codepoint(
             codepoint, follows_joiner, regional_count));
    if (extends) {
      columns.back().byte_end = offset + length;
    } else {
      columns.push_back(TextGraphemeColumn{
          .column_start = columns.size(),
          .column_end = columns.size() + 1U,
          .byte_start = offset,
          .byte_end = offset + length,
          .glyph_start = unset_glyph,
          .glyph_end = unset_glyph,
      });
      regional_count = 0;
    }
    follows_joiner = is_text_measurement_zero_width_joiner(codepoint);
    if (is_text_measurement_regional_indicator(codepoint)) {
      regional_count += 1U;
    }
    previous = codepoint;
    offset += length;
  }
  return columns;
}

std::vector<ShapedCluster> shaped_clusters(const TextShapeRun& run) {
  std::vector<ShapedCluster> clusters;
  clusters.reserve(run.glyphs.size());
  for (std::size_t index = 0; index < run.glyphs.size(); ++index) {
    const TextGlyphRun& glyph = run.glyphs[index];
    const std::size_t end =
        std::min(glyph.byte_offset + glyph.byte_length, run.text.size());
    if (!clusters.empty() && clusters.back().byte_start == glyph.byte_offset) {
      clusters.back().glyph_end = index + 1U;
      clusters.back().byte_end = std::max(clusters.back().byte_end, end);
      clusters.back().advance += glyph.advance;
      continue;
    }
    clusters.push_back(ShapedCluster{
        .byte_start = glyph.byte_offset,
        .byte_end = end,
        .glyph_start = index,
        .glyph_end = index + 1U,
        .advance = glyph.advance,
    });
  }
  return clusters;
}

void assign_cluster(
    std::vector<TextGraphemeColumn>& columns,
    std::size_t first,
    std::size_t last,
    const ShapedCluster& cluster,
    float scale) {
  // A ligature covering several graphemes divides its advance evenly so
  // carets and hit tests can land inside it. The first grapheme owns the
  // glyphs; the rest get an empty range after them.
  const float share = cluster.advance / static_cast<float>(last - first);
  for (std::size_t index = first; index < last; ++index) {
    TextGraphemeColumn& column = columns[index];
    column.advance += share;
    column.device_advance += share * scale;
    if (index == first) {
      if (column.glyph_start == unset_glyph) {
        column.glyph_start = cluster.glyph_start;
      }
      column.glyph_end = cluster.glyph_end;
    } else if (column.glyph_start == unset_glyph) {
      column.glyph_start = cluster.glyph_end;
      column.glyph_end = cluster.glyph_end;
    }
  }
}

} // namespace

std::vector<TextGraphemeColumn> build_text_grapheme_columns(
    const TextShapeRun& run) {
  std::vector<TextGraphemeColumn> columns = segment_graphemes(run.text);
  if (columns.empty()) {
    return columns;
  }
  const float scale = normalized_scale(run.scale);
  std::size_t cursor = 0;
  for (const ShapedCluster& cluster : shaped_clusters(run)) {
    while (cursor < columns.size() &&
           columns[cursor].byte_end <= cluster.byte_start) {
      cursor += 1U;
    }
    std::size_t first = std::min(cursor, columns.size() - 1U);
    std::size_t last = first;
    while (last < columns.size() &&
           columns[last].byte_start < std::max(cluster.byte_end,
                                               cluster.byte_start + 1U)) {
      last += 1U;
    }
    if (last == first) {
      last = first + 1U;
    }
    assign_cluster(columns, first, last, cluster, scale);
  }
  std::size_t previous_end = 0;
  for (TextGraphemeColumn& column : columns) {
    if (column.glyph_start == unset_glyph) {
      column.glyph_start = previous_end;
      column.glyph_end = previous_end;
    }
    previous_end = column.glyph_end;
  }
  return columns;
}

} // namespace cgpui
