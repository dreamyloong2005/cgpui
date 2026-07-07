#include "ui_internal.hpp"

namespace cgpui {
namespace {

[[nodiscard]] bool should_paint_recycled_child(
    const UniformListLayoutSnapshot& snapshot,
    std::size_t index) {
  return !snapshot.items.empty() && snapshot.recycling_window.recycles(index);
}

} // namespace

void ScrollableListElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  paint_list.push_metadata(paint_metadata_for_style(style_));
  if (bounds.has_value()) {
    paint_list.push_clip(*bounds);
  }
  const auto& children = content_.children();
  for (std::size_t index = 0; index < children.size(); ++index) {
    const auto& child = children[index];
    if (should_paint_recycled_child(layout_snapshot_, index)) {
      continue;
    }
    if (child) {
      child->paint(paint_list);
    }
  }
  if (bounds.has_value()) {
    paint_list.pop_clip();
  }
  paint_list.pop_metadata();
}

} // namespace cgpui
