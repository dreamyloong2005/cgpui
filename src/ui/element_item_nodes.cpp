#include "cgpui/ui/element_item_nodes.hpp"

#include "text_style_inheritance.hpp"

#include <utility>

namespace cgpui {

ItemElement::ItemElement(
    ItemKind kind,
    std::string action_name,
    StyleState style_state,
    bool selected,
    ClickHandler click_handler,
    std::unique_ptr<Element> child)
    : kind_(kind),
      action_name_(std::move(action_name)),
      style_state_(std::move(style_state)),
      selected_(selected),
      click_handler_(std::move(click_handler)),
      child_(std::move(child)) {}

ItemKind ItemElement::kind() const {
  return kind_;
}

std::string_view ItemElement::action_name() const {
  return action_name_;
}

const StyleState& ItemElement::style_state() const {
  return style_state_;
}

bool ItemElement::selected() const {
  return selected_;
}

void ItemElement::set_selected(bool selected) {
  selected_ = selected;
}

Element* ItemElement::child() {
  return child_.get();
}

const Element* ItemElement::child() const {
  return child_.get();
}

bool ItemElement::focusable() const {
  return true;
}

AccessibilityRole ItemElement::accessibility_role() const {
  return kind_ == ItemKind::menu_item ? AccessibilityRole::menu_item
                                      : AccessibilityRole::list_item;
}

std::string ItemElement::accessibility_name() const {
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

std::string ItemElement::accessibility_value() const {
  if (kind_ == ItemKind::menu_item) {
    return {};
  }
  return selected_ ? "selected" : "unselected";
}

void ItemElement::inherit_text_style(const Style& style) {
  inherited_text_style_ = inherited_text_style(style);
}

ElementId ItemElement::hit_test(Point point) const {
  const ElementId child_hit = child_ ? child_->hit_test(point) : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

EventResult ItemElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  if (!enabled()) {
    return EventResult::unhandled();
  }
  if (context.gesture != ElementGestureKind::click) {
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

int ItemElement::z_index() const {
  return style_state_.base.z_index;
}

int ItemElement::layer() const {
  return style_state_.base.layer;
}

} // namespace cgpui
