#include "cgpui/ui/element_choice_nodes.hpp"

#include "text_style_inheritance.hpp"

#include <utility>

namespace cgpui {

ToggleControlElement::ToggleControlElement(
    ToggleControlKind kind,
    std::string action_name,
    StyleState style_state,
    bool checked,
    ClickHandler click_handler,
    std::unique_ptr<Element> child)
    : kind_(kind),
      action_name_(std::move(action_name)),
      style_state_(std::move(style_state)),
      checked_(checked),
      click_handler_(std::move(click_handler)),
      child_(std::move(child)) {}

ToggleControlKind ToggleControlElement::kind() const {
  return kind_;
}

std::string_view ToggleControlElement::action_name() const {
  return action_name_;
}

const StyleState& ToggleControlElement::style_state() const {
  return style_state_;
}

bool ToggleControlElement::checked() const {
  return checked_;
}

bool ToggleControlElement::selected() const {
  return checked_;
}

bool ToggleControlElement::on() const {
  return checked_;
}

void ToggleControlElement::set_checked(bool checked) {
  checked_ = checked;
}

Element* ToggleControlElement::child() {
  return child_.get();
}

const Element* ToggleControlElement::child() const {
  return child_.get();
}

bool ToggleControlElement::focusable() const {
  return true;
}

AccessibilityRole ToggleControlElement::accessibility_role() const {
  switch (kind_) {
    case ToggleControlKind::checkbox:
      return AccessibilityRole::checkbox;
    case ToggleControlKind::radio:
      return AccessibilityRole::radio;
    case ToggleControlKind::toggle_switch:
      return AccessibilityRole::switch_control;
  }
  return AccessibilityRole::checkbox;
}

std::string ToggleControlElement::accessibility_name() const {
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

std::string ToggleControlElement::accessibility_value() const {
  if (kind_ == ToggleControlKind::toggle_switch) {
    return checked_ ? "on" : "off";
  }
  return checked_ ? "checked" : "unchecked";
}

void ToggleControlElement::inherit_text_style(const Style& style) {
  inherited_text_style_ = inherited_text_style(style);
}

ElementId ToggleControlElement::hit_test(Point point) const {
  const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

EventResult ToggleControlElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  (void)event;
  if (!enabled()) {
    return EventResult::unhandled();
  }
  if (context.gesture != ElementGestureKind::click) {
    return child_ == nullptr || !child_->enabled()
               ? EventResult::unhandled()
               : child_->handle_event(event, context);
  }

  if (kind_ == ToggleControlKind::radio) {
    checked_ = true;
  } else {
    checked_ = !checked_;
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

int ToggleControlElement::z_index() const {
  return style_state_.base.z_index;
}

int ToggleControlElement::layer() const {
  return style_state_.base.layer;
}

} // namespace cgpui
