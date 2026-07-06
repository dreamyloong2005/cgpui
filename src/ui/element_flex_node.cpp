#include "cgpui/ui/element_layout_nodes.hpp"

#include "element_layer_ordering.hpp"
#include "text_style_inheritance.hpp"

#include <optional>
#include <utility>

namespace cgpui {

FlexElement::FlexElement(FlexDirection direction) : direction_(direction) {}

FlexDirection FlexElement::direction() const {
  return direction_;
}

float FlexElement::gap() const {
  return gap_;
}

void FlexElement::set_gap(float gap) {
  gap_ = gap;
}

AlignItems FlexElement::align_items() const {
  return align_items_;
}

void FlexElement::set_align_items(AlignItems align_items) {
  align_items_ = align_items;
}

JustifyContent FlexElement::justify_content() const {
  return justify_content_;
}

void FlexElement::set_justify_content(JustifyContent justify_content) {
  justify_content_ = justify_content;
}

void FlexElement::append_child(std::unique_ptr<Element> child) {
  if (child) {
    children_.push_back(std::move(child));
  }
}

std::span<const std::unique_ptr<Element>> FlexElement::children() const {
  return children_;
}

void FlexElement::inherit_text_style(const Style& style) {
  inherited_text_style_ = inherited_text_style(style);
}

ElementId FlexElement::hit_test(Point point) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || !contains(*bounds, point)) {
    return {};
  }

  for (const Element* child : hit_test_ordered_children(children_)) {
    const ElementId hit = child->hit_test(point);
    if (hit.value != 0) {
      return hit;
    }
  }
  return id();
}

} // namespace cgpui
