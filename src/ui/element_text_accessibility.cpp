#include "cgpui/ui/element_text_nodes.hpp"

namespace cgpui {

AccessibilityPatternState TextInputElement::accessibility_patterns() const {
  return {.value_settable = true};
}

EventResult TextInputElement::handle_accessibility_action(
    const AccessibilityActionRequested& action,
    const ElementEventContext&) {
  if (!enabled() || action.kind != AccessibilityActionKind::set_value ||
      model() == nullptr) {
    return EventResult::unhandled();
  }
  model()->set_selection(0, text().size());
  model()->insert_text(action.value);
  return EventResult::consumed_event();
}

} // namespace cgpui
