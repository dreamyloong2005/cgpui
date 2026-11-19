#include "cgpui/ui/widget_tree.hpp"

#include <optional>
#include <utility>

namespace cgpui {

WidgetNodeElement::WidgetNodeElement(WidgetNode node, Point parent_origin)
    : node_(std::move(node)),
      offset_{.x = node_.bounds.origin.x - parent_origin.x,
              .y = node_.bounds.origin.y - parent_origin.y} {
  set_enabled(node_.enabled);
}

const WidgetNode& WidgetNodeElement::node() const {
  return node_;
}

LayoutOutput WidgetNodeElement::layout(LayoutInput input) const {
  (void)input;
  set_layout_bounds(Rect{.origin = offset_, .size = node_.bounds.size});
  return LayoutOutput{.origin = offset_, .size = node_.bounds.size};
}

ElementId WidgetNodeElement::hit_test(Point point) const {
  const std::optional<Rect> bounds = layout_bounds();
  return node_.hit_testable && bounds.has_value() && contains(*bounds, point)
             ? id()
             : ElementId{};
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

EventResult WidgetNodeElement::handle_accessibility_action(
    const AccessibilityActionRequested& action,
    const ElementEventContext& context) {
  (void)context;
  return node_.on_action ? node_.on_action(action) : EventResult::unhandled();
}

} // namespace cgpui
