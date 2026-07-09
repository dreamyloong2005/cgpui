#include "cgpui/ui/text_hit_testing.hpp"

#include <algorithm>

namespace cgpui {
namespace {

TextSelectionDragDirection text_selection_drag_direction_for_offsets(
    std::size_t anchor_offset,
    std::size_t head_offset) {
  if (anchor_offset == head_offset) {
    return TextSelectionDragDirection::collapsed;
  }
  return anchor_offset < head_offset ? TextSelectionDragDirection::forward
                                     : TextSelectionDragDirection::backward;
}

} // namespace

TextHitTestResult hit_test_text_position(
    const TextMeasurement& measurement,
    Rect bounds,
    Point point) {
  const TextShapeRun& run = measurement.shape_run;
  const float local_x = point.x - bounds.origin.x;
  const float local_y = point.y - bounds.origin.y;
  const bool inside = contains(bounds, point);

  if (local_x <= 0.0F || run.glyphs.empty()) {
    return TextHitTestResult{
        .byte_offset = 0,
        .local_position = {.x = local_x, .y = local_y},
        .inside_bounds = inside,
    };
  }
  if (local_x >= run.total_advance) {
    return TextHitTestResult{
        .byte_offset = run.byte_length,
        .local_position = {.x = local_x, .y = local_y},
        .inside_bounds = inside,
    };
  }

  float glyph_x = 0.0F;
  for (const TextGlyphRun& glyph : run.glyphs) {
    const float midpoint = glyph_x + (glyph.advance * 0.5F);
    if (local_x <= midpoint) {
      return TextHitTestResult{
          .byte_offset = glyph.byte_offset,
          .local_position = {.x = local_x, .y = local_y},
          .inside_bounds = inside,
      };
    }
    glyph_x += glyph.advance;
  }

  return TextHitTestResult{
      .byte_offset = run.byte_length,
      .local_position = {.x = local_x, .y = local_y},
      .inside_bounds = inside,
  };
}

TextSelectionRange text_selection_range_from_points(
    const TextMeasurement& measurement,
    Rect bounds,
    Point anchor,
    Point head) {
  const std::size_t anchor_offset =
      hit_test_text_position(measurement, bounds, anchor).byte_offset;
  const std::size_t head_offset =
      hit_test_text_position(measurement, bounds, head).byte_offset;
  const std::size_t start = std::min(anchor_offset, head_offset);
  const std::size_t end = std::max(anchor_offset, head_offset);
  return TextSelectionRange{
      .start = start,
      .end = end,
      .collapsed = start == end,
  };
}

TextSelectionDrag text_selection_drag_from_offsets(
    std::size_t anchor_offset,
    std::size_t head_offset) {
  const std::size_t start = std::min(anchor_offset, head_offset);
  const std::size_t end = std::max(anchor_offset, head_offset);
  return TextSelectionDrag{
      .anchor_offset = anchor_offset,
      .head_offset = head_offset,
      .range =
          TextSelectionRange{
              .start = start,
              .end = end,
              .collapsed = start == end,
          },
      .direction =
          text_selection_drag_direction_for_offsets(anchor_offset, head_offset),
  };
}

TextSelectionDrag text_selection_drag_from_points(
    const TextMeasurement& measurement,
    Rect bounds,
    Point anchor,
    Point head) {
  const std::size_t anchor_offset =
      hit_test_text_position(measurement, bounds, anchor).byte_offset;
  const std::size_t head_offset =
      hit_test_text_position(measurement, bounds, head).byte_offset;
  return text_selection_drag_from_offsets(anchor_offset, head_offset);
}

TextSelectionGranularity text_selection_granularity_for_click_count(
    std::uint8_t click_count) {
  if (click_count >= 3U) {
    return TextSelectionGranularity::line;
  }
  if (click_count == 2U) {
    return TextSelectionGranularity::word;
  }
  return TextSelectionGranularity::caret;
}

} // namespace cgpui
