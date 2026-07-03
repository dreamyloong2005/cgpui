#include "cgpui/ui/element_button_nodes.hpp"

#include <utility>
#include <variant>

namespace cgpui {

ButtonElement::ButtonElement(
    std::string action_name,
    StyleState style_state,
    ClickHandler click_handler,
    std::unique_ptr<Element> child)
    : action_name_(std::move(action_name)),
      style_state_(std::move(style_state)),
      click_handler_(std::move(click_handler)),
      child_(std::move(child)) {}

std::string_view ButtonElement::action_name() const {
  return action_name_;
}

const StyleState& ButtonElement::style_state() const {
  return style_state_;
}

Element* ButtonElement::child() {
  return child_.get();
}

const Element* ButtonElement::child() const {
  return child_.get();
}

bool ButtonElement::focusable() const {
  return true;
}

AccessibilityRole ButtonElement::accessibility_role() const {
  return AccessibilityRole::button;
}

std::string ButtonElement::accessibility_name() const {
  if (child_) {
    std::string child_name = child_->accessibility_name();
    if (!child_name.empty()) {
      return child_name;
    }
    std::string child_text = child_->accessibility_text();
    if (!child_text.empty()) {
      return child_text;
    }
  }
  return action_name_;
}

LayoutOutput ButtonElement::layout(LayoutInput input) const {
  Size content_size = style_state_.base.preferred_size;
  if (child_) {
    const LayoutOutput child_output = child_->layout(input);
    content_size = child_output.size;
    child_->set_layout_bounds(Rect{
        .origin =
            {
                .x = style_state_.base.margin.left +
                     style_state_.base.padding.left,
                .y = style_state_.base.margin.top +
                     style_state_.base.padding.top,
            },
        .size = child_output.size,
    });
  }
  const Size preferred{
      .width = content_size.width + style_state_.base.padding.left +
               style_state_.base.padding.right +
               style_state_.base.margin.left + style_state_.base.margin.right,
      .height = content_size.height + style_state_.base.padding.top +
                style_state_.base.padding.bottom +
                style_state_.base.margin.top + style_state_.base.margin.bottom,
  };
  const LayoutOutput output{
      .size = constrain_size(preferred, input.constraints),
  };
  set_layout_bounds(Rect{
      .origin = output.origin,
      .size = output.size,
  });
  return output;
}

ElementId ButtonElement::hit_test(Point point) const {
  const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

EventResult ButtonElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  if (!enabled()) {
    return EventResult::unhandled();
  }
  const auto* pointer_button = std::get_if<PointerButton>(&event);
  if (pointer_button == nullptr || !pointer_button->pressed ||
      pointer_button->button != MouseButton::left) {
    return child_ == nullptr || !child_->enabled()
               ? EventResult::unhandled()
               : child_->handle_event(event, context);
  }

  EventResult result = EventResult::unhandled();
  if (click_handler_) {
    result = click_handler_(context);
    if (result.consumed || result.cancelled) {
      return result;
    }
  }
  if (!action_name_.empty() && context.dispatch_action) {
    result = context.dispatch_action(action_name_);
  }
  return result;
}

int ButtonElement::z_index() const {
  return style_state_.base.z_index;
}

int ButtonElement::layer() const {
  return style_state_.base.layer;
}

} // namespace cgpui
