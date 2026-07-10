#include "styled_box_paint_internal.hpp"

#include <algorithm>
#include <optional>

namespace cgpui {
namespace {

bool has_positive_radius(BorderRadii radius) {
  return radius.top_left > 0.0F || radius.top_right > 0.0F ||
         radius.bottom_right > 0.0F || radius.bottom_left > 0.0F;
}

std::optional<float> uniform_positive_border_width(EdgeSizes width) {
  if (width.top <= 0.0F || width.top != width.right ||
      width.top != width.bottom || width.top != width.left) {
    return std::nullopt;
  }
  return width.top;
}

void paint_nonuniform_border(
    PaintList& paint_list,
    Rect rect,
    Color color,
    EdgeSizes width) {
  const float vertical_side_height =
      std::max(0.0F, rect.size.height - width.top - width.bottom);
  if (width.top > 0.0F) {
    paint_list.fill_rect(
        Rect{
            .origin = rect.origin,
            .size = {.width = rect.size.width, .height = width.top},
        },
        color);
  }
  if (width.right > 0.0F && vertical_side_height > 0.0F) {
    paint_list.fill_rect(
        Rect{
            .origin = {
                .x = rect.origin.x + rect.size.width - width.right,
                .y = rect.origin.y + width.top,
            },
            .size = {.width = width.right, .height = vertical_side_height},
        },
        color);
  }
  if (width.bottom > 0.0F) {
    paint_list.fill_rect(
        Rect{
            .origin = {
                .x = rect.origin.x,
                .y = rect.origin.y + rect.size.height - width.bottom,
            },
            .size = {.width = rect.size.width, .height = width.bottom},
        },
        color);
  }
  if (width.left > 0.0F && vertical_side_height > 0.0F) {
    paint_list.fill_rect(
        Rect{
            .origin = {.x = rect.origin.x, .y = rect.origin.y + width.top},
            .size = {.width = width.left, .height = vertical_side_height},
        },
        color);
  }
}

} // namespace

void paint_styled_box_base(
    PaintList& paint_list,
    const std::optional<Rect>& bounds,
    const Style& style) {
  if (!bounds.has_value()) {
    return;
  }
  if (style.box_shadow.has_value()) {
    paint_list.draw_box_shadow(*bounds, *style.box_shadow, style.border_radius);
  }

  const std::optional<float> uniform_border_width =
      style.border_color.has_value()
          ? uniform_positive_border_width(style.border_width)
          : std::nullopt;
  if (style.background_color.has_value()) {
    if (uniform_border_width.has_value()) {
      paint_list.fill_stroked_rounded_rect(
          *bounds,
          *style.background_color,
          style.border_radius,
          *style.border_color,
          *uniform_border_width);
    } else if (has_positive_radius(style.border_radius)) {
      paint_list.fill_rounded_rect(
          *bounds, *style.background_color, style.border_radius);
    } else {
      paint_list.fill_rect(*bounds, *style.background_color);
    }
  } else if (uniform_border_width.has_value()) {
    paint_list.stroke_rounded_rect(
        *bounds,
        *style.border_color,
        style.border_radius,
        *uniform_border_width);
  }

  if (style.border_color.has_value() && !uniform_border_width.has_value()) {
    paint_nonuniform_border(
        paint_list, *bounds, *style.border_color, style.border_width);
  }
}

} // namespace cgpui
