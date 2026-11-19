#include "cgpui/ui/element_layout_nodes.hpp"

#include "element_layer_ordering.hpp"
#include "styled_box_paint_internal.hpp"
#include "ui_paint_internal.hpp"

namespace cgpui {

namespace {

// Paints the stack's own box (shadow, background, border) beneath its
// children, which paint in z order.
void paint_stack(
    PaintList& paint_list,
    const std::optional<Rect>& bounds,
    const Style& style,
    const std::vector<std::unique_ptr<Element>>& children) {
  paint_list.push_metadata(paint_metadata_for_style(style));
  paint_styled_box_base(paint_list, bounds, style);
  for (const Element* child : paint_ordered_children(children)) {
    child->paint(paint_list);
  }
  paint_list.pop_metadata();
}

} // namespace

void VerticalStackElement::paint(PaintList& paint_list) const {
  paint_stack(paint_list, layout_bounds(), box_style_, children_);
}

void FlexElement::paint(PaintList& paint_list) const {
  paint_stack(paint_list, layout_bounds(), box_style_, children_);
}

} // namespace cgpui
