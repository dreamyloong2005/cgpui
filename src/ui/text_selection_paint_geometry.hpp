#pragma once

#include "cgpui/ui/paint.hpp"

#include <cstddef>
#include <string_view>

namespace cgpui {

[[nodiscard]] Rect text_caret_rect(
    Rect bounds,
    std::string_view text,
    const FontDescriptor& font,
    float font_size,
    DpiScale scale,
    std::size_t byte_offset);

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
