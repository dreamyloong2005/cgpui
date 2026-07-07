#include "cgpui/ui/element_slider_nodes.hpp"

#include "ui_internal.hpp"

namespace cgpui {

void SliderElement::paint(PaintList& paint_list) const {
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
  const Rect track = track_rect();
  paint_list.fill_rect(track, base_style.border_color.value_or(rgb(120, 128, 140)));
  paint_list.fill_rect(
      Rect{
          .origin = track.origin,
          .size = {.width = track.size.width * normalized_value(),
                   .height = track.size.height},
      },
      base_style.foreground_color.value_or(rgb(46, 116, 255)));
  paint_list.fill_rect(
      thumb_rect(),
      base_style.background_color.value_or(rgb(245, 247, 250)));

  if (uses_hidden_overflow_clip) {
    paint_list.pop_clip();
  }
  paint_list.pop_metadata();
}

} // namespace cgpui
