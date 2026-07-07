#include "ui_internal.hpp"

namespace cgpui {

void ToggleControlElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  const Style& base_style = style_state_.base;
  paint_list.push_metadata(paint_metadata_for_style(base_style));
  const bool uses_hidden_overflow_clip =
      bounds.has_value() && base_style.overflow == Overflow::hidden;
  if (uses_hidden_overflow_clip) {
    paint_list.push_clip(base_style.clip_rect.has_value()
                             ? *base_style.clip_rect
                             : *bounds);
  }
  paint_styled_box_base(paint_list, bounds, base_style);
  if (child_) {
    child_->paint(paint_list);
  }
  if (uses_hidden_overflow_clip) {
    paint_list.pop_clip();
  }
  paint_list.pop_metadata();
}

} // namespace cgpui
