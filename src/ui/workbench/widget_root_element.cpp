#include "cgpui/ui/widget_tree.hpp"

#include <limits>

namespace cgpui {

WidgetRootElement::WidgetRootElement(Size viewport) : viewport_(viewport) {
  set_layout_bounds(Rect{.size = viewport_});
}

void WidgetRootElement::attach(const ElementTree* tree) {
  tree_ = tree;
}

LayoutOutput WidgetRootElement::layout(LayoutInput input) const {
  const Size size = constrain_size(viewport_, input.constraints);
  set_layout_bounds(Rect{.size = size});
  return LayoutOutput{.size = size};
}

ElementId WidgetRootElement::hit_test(Point point) const {
  if (tree_ == nullptr) return Element::hit_test(point);
  const ElementId child = hit_test_children(id(), point);
  return child.value != 0 ? child : Element::hit_test(point);
}

// Topmost first: descending z_order, later siblings first within one z.
// Walks distinct z levels instead of sorting so hit tests do not allocate.
ElementId WidgetRootElement::hit_test_children(ElementId id, Point point) const {
  const std::span<const ElementId> children = tree_->children(id);
  long long upper = std::numeric_limits<long long>::max();
  while (true) {
    long long level = std::numeric_limits<long long>::min();
    for (const ElementId child : children) {
      const Element* element = tree_->get(child);
      if (element == nullptr) continue;
      const long long z = element->z_order();
      if (z < upper && z > level) level = z;
    }
    if (level == std::numeric_limits<long long>::min()) return {};
    for (std::size_t index = children.size(); index-- > 0;) {
      const Element* element = tree_->get(children[index]);
      if (element == nullptr || element->z_order() != level) continue;
      const ElementId nested = hit_test_children(children[index], point);
      if (nested.value != 0) return nested;
      const ElementId own = element->hit_test(point);
      if (own.value != 0) return own;
    }
    upper = level;
  }
}

} // namespace cgpui
