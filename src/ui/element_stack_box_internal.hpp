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

} // namespace cgpui
