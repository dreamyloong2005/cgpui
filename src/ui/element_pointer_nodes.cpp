#include "cgpui/ui/element_pointer_nodes.hpp"

#include <utility>
#include <variant>

namespace cgpui {

ClickElement::ClickElement(std::unique_ptr<Element> child, ClickHandler handler)
    : child_(std::move(child)),
      handler_(std::move(handler)) {}

Element* ClickElement::child() {
  return child_.get();
}

const Element* ClickElement::child() const {
  return child_.get();
}

LayoutOutput ClickElement::layout(LayoutInput input) const {
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

void ClickElement::inherit_text_style(const Style& style) {
  if (child_) {
    child_->inherit_text_style(style);
  }
}

ElementId ClickElement::hit_test(Point point) const {
  const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

void ClickElement::paint(PaintList& paint_list) const {
  if (child_) {
    child_->paint(paint_list);
  }
}

EventResult ClickElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  if (!enabled()) {
    return EventResult::unhandled();
  }
  if (const auto* pointer_button = std::get_if<PointerButton>(&event);
      pointer_button != nullptr && pointer_button->pressed && handler_) {
    return handler_(context);
  }
  return child_ == nullptr || !child_->enabled()
             ? EventResult::unhandled()
             : child_->handle_event(event, context);
}

PointerElement::PointerElement(
    std::unique_ptr<Element> child,
    PointerButtonHandler down_handler,
    PointerButtonHandler up_handler,
    PointerMoveHandler move_handler)
    : child_(std::move(child)),
      down_handler_(std::move(down_handler)),
      up_handler_(std::move(up_handler)),
      move_handler_(std::move(move_handler)) {}

Element* PointerElement::child() {
  return child_.get();
}

const Element* PointerElement::child() const {
  return child_.get();
}

LayoutOutput PointerElement::layout(LayoutInput input) const {
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

void PointerElement::inherit_text_style(const Style& style) {
  if (child_) {
    child_->inherit_text_style(style);
  }
}

ElementId PointerElement::hit_test(Point point) const {
  const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

void PointerElement::paint(PaintList& paint_list) const {
  if (child_) {
    child_->paint(paint_list);
  }
}

EventResult PointerElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  if (!enabled()) {
    return EventResult::unhandled();
  }

  if (const auto* pointer_button = std::get_if<PointerButton>(&event);
      pointer_button != nullptr) {
    PointerButtonHandler& handler =
        pointer_button->pressed ? down_handler_ : up_handler_;
    if (handler) {
      const EventResult result = handler(*pointer_button, context);
      if (result.consumed || result.cancelled) {
        return result;
      }
    }
  } else if (const auto* pointer_move = std::get_if<PointerMoved>(&event);
             pointer_move != nullptr && move_handler_) {
    const EventResult result = move_handler_(*pointer_move, context);
    if (result.consumed || result.cancelled) {
      return result;
    }
  }

  return child_ == nullptr || !child_->enabled()
             ? EventResult::unhandled()
             : child_->handle_event(event, context);
}

} // namespace cgpui
