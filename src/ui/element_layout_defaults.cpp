#include "cgpui/ui/element_core.hpp"

namespace cgpui {

LayoutOutput Element::layout(LayoutInput input) const {
  const LayoutOutput output{
      .size = constrain_size({}, input.constraints),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  return output;
}

std::optional<Rect> Element::layout_bounds() const {
  return layout_bounds_;
}

ElementId Element::hit_test(Point point) const {
  return layout_bounds_.has_value() && contains(*layout_bounds_, point)
      ? id_ : ElementId{};
}

void Element::set_layout_bounds(Rect bounds) const {
  layout_bounds_ = bounds;
}

void Element::offset_layout_children(Point delta) const {
  (void)delta;
}

Point absolute_origin(Size parent_size, EdgeSizes inset) {
  (void)parent_size;
  return Point{.x = inset.left, .y = inset.top};
}

} // namespace cgpui
