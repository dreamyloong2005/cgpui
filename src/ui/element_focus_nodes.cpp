#include "cgpui/ui/element_focus_nodes.hpp"

#include <utility>
#include <variant>

namespace cgpui {

ChildViewElement::ChildViewElement(ViewId view_id, Size placeholder_size)
    : view_id_(view_id),
      placeholder_size_(placeholder_size) {}

ViewId ChildViewElement::view_id() const {
  return view_id_;
}

Size ChildViewElement::placeholder_size() const {
  return placeholder_size_;
}

LayoutOutput ChildViewElement::layout(LayoutInput input) const {
  const LayoutOutput output{
      .size = constrain_size(placeholder_size_, input.constraints),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  return output;
}

FocusableElement::FocusableElement(std::unique_ptr<Element> child)
    : child_(std::move(child)) {}

Element* FocusableElement::child() {
  return child_.get();
}

const Element* FocusableElement::child() const {
  return child_.get();
}

bool FocusableElement::focusable() const {
  return true;
}

LayoutOutput FocusableElement::layout(LayoutInput input) const {
  if (child_) {
    const LayoutOutput output = child_->layout(input);
    child_->set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }
  return Element::layout(input);
}

ElementId FocusableElement::hit_test(Point point) const {
  const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

void FocusableElement::paint(PaintList& paint_list) const {
  if (child_) {
    child_->paint(paint_list);
  }
}

EventResult FocusableElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  return !enabled() || child_ == nullptr || !child_->enabled()
             ? EventResult::unhandled()
             : child_->handle_event(event, context);
}

KeyElement::KeyElement(std::unique_ptr<Element> child, KeyHandler handler)
    : child_(std::move(child)),
      handler_(std::move(handler)) {}

Element* KeyElement::child() {
  return child_.get();
}

const Element* KeyElement::child() const {
  return child_.get();
}

LayoutOutput KeyElement::layout(LayoutInput input) const {
  if (child_) {
    const LayoutOutput output = child_->layout(input);
    child_->set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }
  return Element::layout(input);
}

ElementId KeyElement::hit_test(Point point) const {
  const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

void KeyElement::paint(PaintList& paint_list) const {
  if (child_) {
    child_->paint(paint_list);
  }
}

EventResult KeyElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  if (!enabled()) {
    return EventResult::unhandled();
  }
  if (const auto* key = std::get_if<KeyboardKey>(&event);
      key != nullptr && handler_) {
    return handler_(*key, context);
  }
  return child_ == nullptr || !child_->enabled()
             ? EventResult::unhandled()
             : child_->handle_event(event, context);
}

} // namespace cgpui
