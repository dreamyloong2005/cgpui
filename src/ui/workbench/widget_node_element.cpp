#include "cgpui/ui/widget_tree.hpp"

#include <utility>

namespace cgpui {

WidgetNodeElement::WidgetNodeElement(WidgetNode node) : node_(std::move(node)) {
  set_enabled(node_.enabled);
  set_layout_bounds(node_.bounds);
}

const WidgetNode& WidgetNodeElement::node() const {
  return node_;
}

LayoutOutput WidgetNodeElement::layout(LayoutInput input) const {
  (void)input;
  set_layout_bounds(node_.bounds);
  return LayoutOutput{.origin = node_.bounds.origin, .size = node_.bounds.size};
}

ElementId WidgetNodeElement::hit_test(Point point) const {
  return node_.hit_testable && contains(node_.bounds, point) ? id() : ElementId{};
}

bool WidgetNodeElement::focusable() const {
  return node_.focusable;
}

int WidgetNodeElement::z_index() const {
  return node_.z_order;
}

AccessibilityRole WidgetNodeElement::accessibility_role() const {
  return node_.role;
}

std::string WidgetNodeElement::accessibility_name() const {
  return node_.name;
}

std::string WidgetNodeElement::accessibility_value() const {
  return node_.value;
}

AccessibilityPatternState WidgetNodeElement::accessibility_patterns() const {
  return node_.patterns;
}

} // namespace cgpui
