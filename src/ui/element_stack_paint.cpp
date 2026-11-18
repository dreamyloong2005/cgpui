#include "cgpui/ui/element_layout_nodes.hpp"

#include "element_layer_ordering.hpp"

namespace cgpui {

// Stacks have no box of their own; they paint their children in z order.
void VerticalStackElement::paint(PaintList& paint_list) const {
  for (const Element* child : paint_ordered_children(children_)) {
    child->paint(paint_list);
  }
}

void FlexElement::paint(PaintList& paint_list) const {
  for (const Element* child : paint_ordered_children(children_)) {
    child->paint(paint_list);
  }
}

} // namespace cgpui
