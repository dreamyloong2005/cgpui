#pragma once

#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/style_box.hpp"

namespace cgpui {

// The box of a v_stack / row / column built with a style: CSS content box
// (a positive preferred size fixes that axis, else the children's extent),
// plus padding and margin, constrained by the parent and the style's
// min / max size.
struct StackBox {
  Size outer;
  Size content;
  // Content box origin relative to the stack's own origin.
  Point content_offset;
};

[[nodiscard]] StackBox resolve_stack_box(
    Size children_extent,
    const Style& style,
    LayoutConstraints constraints);

// The content box of an element laid out with resolve_stack_box at
// `outer`: inset by the style's margin and padding.
[[nodiscard]] Rect stack_box_content_rect(Rect outer, const Style& style);

// The width left for content inside `max_width` once the style's padding
// and margin are taken; a positive preferred width fixes it.
[[nodiscard]] float stack_box_content_max_width(
    float max_width,
    const Style& style);

} // namespace cgpui
