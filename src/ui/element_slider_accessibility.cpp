#include "cgpui/ui/element_slider_nodes.hpp"

#include <algorithm>
#include <sstream>

namespace cgpui {

std::string SliderElement::accessibility_value() const {
  std::ostringstream output;
  output << value_;
  return output.str();
}

AccessibilityPatternState SliderElement::accessibility_patterns() const {
  const double span = static_cast<double>(max_ - min_);
  const double small = step_ > 0.0F ? step_ : span / 100.0;
  return {.range = AccessibilityRangeValue{
              .value = value_,
              .minimum = min_,
              .maximum = max_,
              .small_change = small,
              .large_change = std::max(small, span / 10.0),
          }};
}

EventResult SliderElement::handle_accessibility_action(
    const AccessibilityActionRequested& action,
    const ElementEventContext& context) {
  if (!enabled() || action.kind != AccessibilityActionKind::set_range_value) {
    return EventResult::unhandled();
  }
  const float next_value = clamped_value(
      static_cast<float>(action.numeric_value));
  if (next_value == value_) return EventResult::consumed_event();
  value_ = next_value;
  EventResult result = EventResult::unhandled();
  if (change_handler_) {
    result = change_handler_(value_, context);
    if (result.consumed || result.cancelled) return result;
  }
  return !action_name_.empty() && context.dispatch_action
      ? context.dispatch_action(action_name_)
      : EventResult::consumed_event();
}

} // namespace cgpui
