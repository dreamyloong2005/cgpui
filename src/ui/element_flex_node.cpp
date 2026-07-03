#include "cgpui/ui/element_layout_nodes.hpp"

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

ElementId FlexElement::hit_test(Point point) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || !contains(*bounds, point)) {
    return {};
  }

  for (auto iterator = children_.rbegin(); iterator != children_.rend();
       ++iterator) {
    const ElementId hit = (*iterator)->hit_test(point);
    if (hit.value != 0) {
      return hit;
    }
  }
  return id();
}

} // namespace cgpui
