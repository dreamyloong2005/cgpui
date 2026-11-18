#pragma once

#include "cgpui/ui/paint.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

namespace cgpui {

[[nodiscard]] Rect text_caret_rect(
    Rect bounds,
    std::string_view text,
    const FontDescriptor& font,
    float font_size,
    DpiScale scale,
    std::size_t byte_offset);

// One wrapped line's part of a byte range: the line-height rect and the
// absolute baseline y, plus the primary face metrics of the run.
struct TextRangeLineRect {
  Rect rect;
  float baseline = 0.0F;
  TextFontMetrics font_metrics;
};

[[nodiscard]] std::vector<TextRangeLineRect> text_range_line_rects(
    Rect bounds,
    std::string_view text,
    const FontDescriptor& font,
    float font_size,
    DpiScale scale,
    std::size_t start,
    std::size_t end);

void paint_text_selection_ranges(
    PaintList& paint_list,
    Rect bounds,
    Color color,
    std::string_view text,
    const FontDescriptor& font,
    float font_size,
    DpiScale scale,
    TextSelectionRange selection);

void paint_text_caret_geometry(
    PaintList& paint_list,
    Rect bounds,
    Color color,
    std::string_view text,
    const FontDescriptor& font,
    float font_size,
    DpiScale scale,
    std::size_t byte_offset);

} // namespace cgpui
