#include "cgpui/ui/element_core.hpp"

namespace cgpui {

bool Element::focusable() const {
  return false;
}

AccessibilityRole Element::accessibility_role() const {
  return AccessibilityRole::generic;
}

std::string Element::accessibility_name() const {
  return {};
}

std::string Element::accessibility_text() const {
  return {};
}

std::string Element::accessibility_value() const {
  return {};
}

AccessibilityPatternState Element::accessibility_patterns()
    const {
  return {};
}

EventResult Element::handle_accessibility_action(
    const AccessibilityActionRequested& action,
    const ElementEventContext& context) {
  (void)action;
  (void)context;
  return EventResult::unhandled();
}

} // namespace cgpui
