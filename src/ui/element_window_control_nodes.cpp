#include "cgpui/ui/element_window_control_nodes.hpp"

#include "cgpui/ui/paint.hpp"

#include <utility>

namespace cgpui {

WindowControlAreaElement::WindowControlAreaElement(
    std::unique_ptr<Element> child, WindowControlArea area)
    : child_(std::move(child)), area_(area) {}

WindowControlArea WindowControlAreaElement::area() const {
  return area_;
}

Element* WindowControlAreaElement::child() {
  return child_.get();
}

const Element* WindowControlAreaElement::child() const {
  return child_.get();
}

LayoutOutput WindowControlAreaElement::layout(LayoutInput input) const {
  if (!child_) return Element::layout(input);
  const LayoutOutput output = child_->layout(input);
  const Rect bounds{.origin = output.origin, .size = output.size};
  child_->place_layout(bounds);
  set_layout_bounds(bounds);
  return output;
}

void WindowControlAreaElement::inherit_text_style(const Style& style) {
  if (child_) child_->inherit_text_style(style);
}

ElementId WindowControlAreaElement::hit_test(Point point) const {
  const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

void WindowControlAreaElement::paint(PaintList& paint_list) const {
  // Use the child's bounds: a parent layout may move the child after this
  // wrapper's own layout ran.
  const Element* source = child_ ? child_.get() : this;
  if (const std::optional<Rect> bounds = source->layout_bounds()) {
    paint_list.add_window_control_region(area_, *bounds);
  }
  if (child_) child_->paint(paint_list);
}

EventResult WindowControlAreaElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  return child_ == nullptr || !child_->enabled()
      ? EventResult::unhandled()
      : child_->handle_event(event, context);
}

} // namespace cgpui
