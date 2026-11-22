#include "cgpui/renderer/planning/draw_geometry.hpp"

namespace cgpui {

std::vector<RoundedRectTessellationRecord> vulkan_tessellate_rounded_rects(
    std::span<const RoundedRectDraw> rounded_rects) {
  return tessellate_rounded_rects(rounded_rects);
}

std::vector<TextSelectionGeometryRecord> vulkan_build_text_selection_geometry(
    std::span<const TextSelectionDraw> selections) {
  return build_text_selection_geometry(selections);
}

std::vector<TextCaretGeometryRecord> vulkan_build_text_caret_geometry(
    std::span<const TextCaretDraw> carets) {
  return build_text_caret_geometry(carets);
}

} // namespace cgpui
