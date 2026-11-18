#include "cgpui/ui/splitter.hpp"

#include "cgpui/ui/paint.hpp"
#include "workbench_internal.hpp"

#include <cmath>
#include <string>

namespace cgpui {

void Splitter::append_elements(
    WidgetTreeBuilder& builder,
    WidgetTreeBuilder::NodeHandle parent) {
  const bool horizontal = options_.axis == SplitterAxis::horizontal;
  const CursorShape cursor = horizontal ? CursorShape::resize_left_right
                                        : CursorShape::resize_up_down;
  const WidgetTreeBuilder::NodeHandle container = builder.add(
      parent,
      WidgetNode{.role = AccessibilityRole::generic,
                 .name = options_.name,
                 .bounds = bounds_,
                 .z_order = 10},
      true);
  const std::optional<ElementId> focused = builder.focused_element();
  focused_.reset();
  const float start = main_of(bounds_.origin);
  const float extent = std::max(1.0F, main_extent());
  for (std::size_t index = 0; index < divider_rects_.size(); ++index) {
    if (focused.has_value() && divider_ids_[index].value != 0 &&
        *focused == divider_ids_[index]) {
      focused_ = index;
    }
    const double percent =
        (main_of(divider_rects_[index].origin) - start) / extent * 100.0;
    const double rounded = std::round(percent);
    builder.add(
        container,
        WidgetNode{
            .role = AccessibilityRole::separator,
            .name = options_.name + " divider " + std::to_string(index + 1),
            .value = std::to_string(static_cast<int>(rounded)) + "%",
            .patterns = {.range = AccessibilityRangeValue{
                             .value = rounded,
                             .maximum = 100.0,
                             .small_change = options_.keyboard_step / extent * 100.0,
                             .large_change =
                                 options_.keyboard_large_step / extent * 100.0}},
            .bounds = divider_hit_rect(index),
            .focusable = true,
            .hit_testable = true,
            .cursor = cursor},
        true,
        &divider_ids_[index]);
  }
  // While dragging, a window-sized shield keeps the resize cursor and keeps
  // hover effects underneath from flickering.
  const bool dragging = dragging_.has_value();
  builder.add(
      parent,
      WidgetNode{.bounds = dragging ? Rect{.size = builder.viewport()} : Rect{},
                 .hit_testable = dragging,
                 .z_order = 900,
                 .cursor = cursor},
      true,
      &shield_id_);
}

void Splitter::paint(PaintList& paint_list, const WorkbenchTheme& theme) const {
  const bool horizontal = options_.axis == SplitterAxis::horizontal;
  const float scale = workbench_detail::scale_of(scale_);
  for (std::size_t index = 0; index < divider_rects_.size(); ++index) {
    const bool active = dragging_ == index || hovered_ == index;
    const bool focused = focused_ == index;
    Rect line = divider_rects_[index];
    paint_list.fill_rect(line, theme.divider);
    if (!active && !focused) continue;
    // Highlight: an accent band of 3 logical pixels (at least 2 device
    // pixels) centered on the divider.
    const float band = std::max(2.0F / scale, workbench_detail::snap(3.0F, scale_));
    if (horizontal) {
      line.origin.x = workbench_detail::snap(
          line.origin.x + line.size.width / 2.0F - band / 2.0F, scale_);
      line.size.width = band;
    } else {
      line.origin.y = workbench_detail::snap(
          line.origin.y + line.size.height / 2.0F - band / 2.0F, scale_);
      line.size.height = band;
    }
    paint_list.fill_rect(line, focused && !active ? theme.focus_ring : theme.accent);
  }
}

} // namespace cgpui
