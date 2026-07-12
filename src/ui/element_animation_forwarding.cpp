#include "cgpui/ui/element_animation.hpp"

namespace cgpui {

void AnimationElement::inherit_text_style(const Style& style) {
  if (child_ != nullptr) child_->inherit_text_style(style);
}

ElementId AnimationElement::hit_test(Point point) const {
  const ElementId child_hit = child_ != nullptr ? child_->hit_test(point)
                                                : ElementId{};
  return child_hit.value != 0 ? child_hit : Element::hit_test(point);
}

void AnimationElement::paint(PaintList& paint_list) const {
  if (child_ != nullptr) child_->paint(paint_list);
}

bool AnimationElement::focusable() const {
  return child_ != nullptr && child_->focusable();
}

AccessibilityRole AnimationElement::accessibility_role() const {
  return child_ != nullptr ? child_->accessibility_role()
                           : AccessibilityRole::generic;
}

std::string AnimationElement::accessibility_name() const {
  return child_ != nullptr ? child_->accessibility_name() : std::string{};
}

std::string AnimationElement::accessibility_text() const {
  return child_ != nullptr ? child_->accessibility_text() : std::string{};
}

std::string AnimationElement::accessibility_value() const {
  return child_ != nullptr ? child_->accessibility_value() : std::string{};
}

AccessibilityPatternState AnimationElement::accessibility_patterns() const {
  return child_ != nullptr ? child_->accessibility_patterns()
                           : AccessibilityPatternState{};
}

EventResult AnimationElement::handle_accessibility_action(
    const AccessibilityActionRequested& action,
    const ElementEventContext& context) {
  return child_ != nullptr
             ? child_->handle_accessibility_action(action, context)
             : EventResult::unhandled();
}

void AnimationElement::focus(const ElementFocusContext& context) {
  if (child_ != nullptr) child_->focus(context);
}

void AnimationElement::on_mount(const ElementLifecycleContext& context) {
  if (child_ != nullptr) child_->on_mount(context);
}

void AnimationElement::on_update(const ElementLifecycleContext& context) {
  if (child_ != nullptr) child_->on_update(context);
}

void AnimationElement::on_unmount(const ElementLifecycleContext& context) {
  if (child_ != nullptr) child_->on_unmount(context);
}

EventResult AnimationElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  return !enabled() || child_ == nullptr || !child_->enabled()
             ? EventResult::unhandled()
             : child_->handle_event(event, context);
}

int AnimationElement::z_index() const {
  return child_ != nullptr ? child_->z_index() : Element::z_index();
}

int AnimationElement::layer() const {
  return child_ != nullptr ? child_->layer() : Element::layer();
}

} // namespace cgpui
