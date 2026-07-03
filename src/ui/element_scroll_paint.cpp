#include "ui_internal.hpp"

namespace cgpui {

void ScrollableListElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  paint_list.push_metadata(paint_metadata_for_style(style_));
  if (bounds.has_value()) {
    paint_list.push_clip(*bounds);
  }
  for (const auto& child : content_.children()) {
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
