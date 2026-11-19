#include "element_stack_box_internal.hpp"

#include <algorithm>

namespace cgpui {

StackBox resolve_stack_box(
    Size children_extent,
    const Style& style,
    LayoutConstraints constraints) {
  Size content = children_extent;
  if (style.preferred_size.width > 0.0F) {
    content.width = style.preferred_size.width;
  }
  if (style.preferred_size.height > 0.0F) {
    content.height = style.preferred_size.height;
  }
  const float inset_x = style.padding.left + style.padding.right +
                        style.margin.left + style.margin.right;
  const float inset_y = style.padding.top + style.padding.bottom +
                        style.margin.top + style.margin.bottom;
  constraints.min_size.width =
      std::max(constraints.min_size.width, style.min_size.width);
  constraints.min_size.height =
      std::max(constraints.min_size.height, style.min_size.height);
  constraints.max_size.width = std::max(
      constraints.min_size.width,
      std::min(constraints.max_size.width, style.max_size.width));
  constraints.max_size.height = std::max(
      constraints.min_size.height,
      std::min(constraints.max_size.height, style.max_size.height));
  const Size outer = constrain_size(
      Size{.width = content.width + inset_x,
           .height = content.height + inset_y},
      constraints);
  return StackBox{
      .outer = outer,
      .content = {.width = std::max(0.0F, outer.width - inset_x),
                  .height = std::max(0.0F, outer.height - inset_y)},
      .content_offset = {.x = style.margin.left + style.padding.left,
                         .y = style.margin.top + style.padding.top},
  };
}

Rect stack_box_content_rect(Rect outer, const Style& style) {
  const float inset_x = style.padding.left + style.padding.right +
                        style.margin.left + style.margin.right;
  const float inset_y = style.padding.top + style.padding.bottom +
                        style.margin.top + style.margin.bottom;
  return Rect{
      .origin = {.x = outer.origin.x + style.margin.left + style.padding.left,
                 .y = outer.origin.y + style.margin.top + style.padding.top},
      .size = {.width = std::max(0.0F, outer.size.width - inset_x),
               .height = std::max(0.0F, outer.size.height - inset_y)},
  };
}

float stack_box_content_max_width(float max_width, const Style& style) {
  if (style.preferred_size.width > 0.0F) {
    return style.preferred_size.width;
  }
  const float inset_x = style.padding.left + style.padding.right +
                        style.margin.left + style.margin.right;
  return std::max(0.0F, max_width - inset_x);
}

} // namespace cgpui
