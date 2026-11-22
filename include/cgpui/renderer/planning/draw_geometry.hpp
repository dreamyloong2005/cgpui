#pragma once

#include "cgpui/renderer/renderer_geometry_reports.hpp"

namespace cgpui {

std::vector<RoundedRectTessellationRecord> tessellate_rounded_rects(
    std::span<const RoundedRectDraw> rounded_rects);
std::vector<TextSelectionGeometryRecord> build_text_selection_geometry(
    std::span<const TextSelectionDraw> selections);
std::vector<TextCaretGeometryRecord> build_text_caret_geometry(
    std::span<const TextCaretDraw> carets);

} // namespace cgpui
