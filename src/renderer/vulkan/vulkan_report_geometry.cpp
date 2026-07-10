#include "vulkan_report_internal.hpp"

#include <cstddef>

namespace cgpui {
namespace {

[[nodiscard]] std::size_t positive_rounded_rect_corner_count(
    BorderRadii radius) {
  std::size_t count = 0;
  if (radius.top_left > 0.0F) {
    ++count;
  }
  if (radius.top_right > 0.0F) {
    ++count;
  }
  if (radius.bottom_right > 0.0F) {
    ++count;
  }
  if (radius.bottom_left > 0.0F) {
    ++count;
  }
  return count;
}

} // namespace

std::vector<RoundedRectTessellationRecord> vulkan_tessellate_rounded_rects(
    std::span<const RoundedRectDraw> rounded_rects) {
  std::vector<RoundedRectTessellationRecord> records;
  records.reserve(rounded_rects.size());

  for (const RoundedRectDraw& rounded_rect : rounded_rects) {
    constexpr std::size_t kCornerSegmentCount = 4;
    const std::size_t rounded_corner_count =
        positive_rounded_rect_corner_count(rounded_rect.radius);
    records.push_back(RoundedRectTessellationRecord{
        .rect = rounded_rect.rect,
        .color = rounded_rect.color,
        .radius = rounded_rect.radius,
        .fill_enabled = rounded_rect.fill_enabled,
        .border_color = rounded_rect.border_color,
        .border_width = rounded_rect.border_width,
        .clip_rect = rounded_rect.clip_rect,
        .clip_stack = rounded_rect.clip_stack,
        .composition_stack = rounded_rect.composition_stack,
        .metadata = rounded_rect.metadata,
        .corner_segment_count =
            rounded_corner_count == 0 ? 0 : kCornerSegmentCount,
        .vertex_count = 4 + rounded_corner_count * kCornerSegmentCount,
        .triangle_count = 2 + rounded_corner_count * kCornerSegmentCount,
    });
  }

  return records;
}

std::vector<TextSelectionGeometryRecord> vulkan_build_text_selection_geometry(
    std::span<const TextSelectionDraw> selections) {
  std::vector<TextSelectionGeometryRecord> records;
  records.reserve(selections.size());

  for (const TextSelectionDraw& selection : selections) {
    records.push_back(TextSelectionGeometryRecord{
        .rect = selection.rect,
        .color = selection.color,
        .range = selection.range,
        .font_size = selection.font_size,
        .clip_rect = selection.clip_rect,
        .clip_stack = selection.clip_stack,
        .composition_stack = selection.composition_stack,
        .metadata = selection.metadata,
        .vertex_count = 4,
        .triangle_count = 2,
    });
  }

  return records;
}

std::vector<TextCaretGeometryRecord> vulkan_build_text_caret_geometry(
    std::span<const TextCaretDraw> carets) {
  std::vector<TextCaretGeometryRecord> records;
  records.reserve(carets.size());

  for (const TextCaretDraw& caret : carets) {
    records.push_back(TextCaretGeometryRecord{
        .rect = caret.rect,
        .color = caret.color,
        .byte_offset = caret.byte_offset,
        .font_size = caret.font_size,
        .clip_rect = caret.clip_rect,
        .clip_stack = caret.clip_stack,
        .composition_stack = caret.composition_stack,
        .metadata = caret.metadata,
        .vertex_count = 4,
        .triangle_count = 2,
    });
  }

  return records;
}

} // namespace cgpui
