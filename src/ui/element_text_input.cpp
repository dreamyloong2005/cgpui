#include "cgpui/ui/element_text_nodes.hpp"

#include <utility>
#include <variant>

namespace cgpui {

bool TextInputElement::focusable() const {
  return true;
}

AccessibilityRole TextInputElement::accessibility_role() const {
  return AccessibilityRole::text_input;
}

std::string TextInputElement::accessibility_value() const {
  return std::string(text());
}

EventResult TextInputElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  (void)context;
  if (!enabled()) {
    return EventResult::unhandled();
  }
  if (std::holds_alternative<TextInput>(event) ||
      std::holds_alternative<ImeComposition>(event)) {
    return EventResult::consumed_event();
  }
  if (const auto* pointer = std::get_if<PointerButton>(&event);
      pointer != nullptr && pointer->pressed &&
      pointer->button == MouseButton::left) {
    return EventResult::consumed_event();
  }
  return EventResult::unhandled();
}

TextInputElement::TextInputElement(TextModel& model, Style style)
    : TextElement(&model, std::move(style)) {}

} // namespace cgpui
