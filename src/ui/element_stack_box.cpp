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

} // namespace cgpui
