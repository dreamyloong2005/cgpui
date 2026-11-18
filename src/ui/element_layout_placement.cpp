#include "cgpui/ui/element_animation.hpp"
#include "cgpui/ui/element_button_nodes.hpp"
#include "cgpui/ui/element_choice_nodes.hpp"
#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/element_focus_nodes.hpp"
#include "cgpui/ui/element_item_nodes.hpp"
#include "cgpui/ui/element_layout_nodes.hpp"
#include "cgpui/ui/element_pointer_nodes.hpp"
#include "cgpui/ui/element_scroll_nodes.hpp"
#include "cgpui/ui/element_style_nodes.hpp"
#include "cgpui/ui/element_window_control_nodes.hpp"

#include <memory>
#include <vector>

namespace cgpui {
namespace {

void offset_each(
    const std::vector<std::unique_ptr<Element>>& children,
    Point delta) {
  for (const auto& child : children) {
    if (child) child->offset_layout(delta);
  }
}

void offset_one(const std::unique_ptr<Element>& child, Point delta) {
  if (child) child->offset_layout(delta);
}

} // namespace

void Element::place_layout(Rect bounds) const {
  const Point previous =
      layout_bounds_.has_value() ? layout_bounds_->origin : Point{};
  layout_bounds_ = bounds;
  const Point delta{
      .x = bounds.origin.x - previous.x,
      .y = bounds.origin.y - previous.y,
  };
  if (delta.x != 0.0F || delta.y != 0.0F) {
    offset_layout_children(delta);
  }
}

void Element::offset_layout(Point delta) const {
  if (layout_bounds_.has_value()) {
    layout_bounds_->origin.x += delta.x;
    layout_bounds_->origin.y += delta.y;
  }
  offset_layout_children(delta);
}

void StyledElement::offset_layout_children(Point delta) const {
  offset_each(children_, delta);
}

void VerticalStackElement::offset_layout_children(Point delta) const {
  offset_each(children_, delta);
}

void FlexElement::offset_layout_children(Point delta) const {
  offset_each(children_, delta);
}

void ButtonElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

void ToggleControlElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

void ItemElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

void FocusableElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

void KeyElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

void ClickElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

void PointerElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

void AnimationElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

void WindowControlAreaElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

void ScrollElement::offset_layout_children(Point delta) const {
  offset_one(child_, delta);
}

} // namespace cgpui
