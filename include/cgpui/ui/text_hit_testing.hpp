#pragma once

#include "cgpui/ui/text_measurement.hpp"

#include <cstddef>

namespace cgpui {

struct TextSelectionRange {
  std::size_t start = 0;
  std::size_t end = 0;
  bool collapsed = true;
};

enum class TextSelectionDragDirection {
  collapsed,
  forward,
  backward,
};

struct TextSelectionDrag {
  std::size_t anchor_offset = 0;
  std::size_t head_offset = 0;
  TextSelectionRange range;
  TextSelectionDragDirection direction =
      TextSelectionDragDirection::collapsed;
};

struct TextHitTestResult {
  std::size_t byte_offset = 0;
  Point local_position;
  bool inside_bounds = false;
};

[[nodiscard]] TextHitTestResult hit_test_text_position(
    const TextMeasurement& measurement,
    Rect bounds,
    Point point);

[[nodiscard]] TextSelectionRange text_selection_range_from_points(
    const TextMeasurement& measurement,
    Rect bounds,
    Point anchor,
    Point head);

[[nodiscard]] TextSelectionDrag text_selection_drag_from_offsets(
    std::size_t anchor_offset,
    std::size_t head_offset);

[[nodiscard]] TextSelectionDrag text_selection_drag_from_points(
    const TextMeasurement& measurement,
    Rect bounds,
    Point anchor,
    Point head);

} // namespace cgpui
