#include "cgpui/ui/element_slider_nodes.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>
#include <variant>

namespace cgpui {

namespace {

std::string format_slider_value(float value) {
  std::ostringstream output;
  output << value;
  return output.str();
}

} // namespace

SliderElement::SliderElement(
    std::string action_name,
    StyleState style_state,
    float min,
    float max,
    float value,
    float step,
    SliderChangeHandler change_handler)
    : action_name_(std::move(action_name)),
      style_state_(std::move(style_state)),
      min_(std::min(min, max)),
      max_(std::max(min, max)),
      step_(step > 0.0F ? step : 0.0F),
      change_handler_(std::move(change_handler)) {
  value_ = clamped_value(value);
}

std::string_view SliderElement::action_name() const {
  return action_name_;
}

const StyleState& SliderElement::style_state() const {
  return style_state_;
}

float SliderElement::min() const {
  return min_;
}

float SliderElement::max() const {
  return max_;
}

float SliderElement::value() const {
  return value_;
}

float SliderElement::step() const {
  return step_;
}

float SliderElement::normalized_value() const {
  const float span = max_ - min_;
  if (span <= 0.0F) {
    return 0.0F;
  }
  return std::clamp((value_ - min_) / span, 0.0F, 1.0F);
}

void SliderElement::set_value(float value) {
  value_ = clamped_value(value);
}

bool SliderElement::focusable() const {
  return true;
}

AccessibilityRole SliderElement::accessibility_role() const {
  return AccessibilityRole::slider;
}

std::string SliderElement::accessibility_name() const {
  return action_name_;
}

std::string SliderElement::accessibility_value() const {
  return format_slider_value(value_);
}

ElementId SliderElement::hit_test(Point point) const {
  return Element::hit_test(point);
}

EventResult SliderElement::handle_event(
    const PlatformEvent& event,
    const ElementEventContext& context) {
  if (!enabled() || context.gesture != ElementGestureKind::click) {
    return EventResult::unhandled();
  }

  const auto* pointer = std::get_if<PointerButton>(&event);
  if (pointer == nullptr || pointer->button != MouseButton::left ||
      pointer->pressed) {
    return EventResult::unhandled();
  }

  const float next_value = value_for_point(pointer->position);
  if (next_value == value_) {
    return EventResult::consumed_event();
  }
  value_ = next_value;

  EventResult result = EventResult::unhandled();
  if (change_handler_) {
    result = change_handler_(value_, context);
    if (result.consumed || result.cancelled) {
      return result;
    }
  }
  if (!action_name_.empty() && context.dispatch_action) {
    return context.dispatch_action(action_name_);
  }
  return EventResult::consumed_event();
}

int SliderElement::z_index() const {
  return style_state_.base.z_index;
}

int SliderElement::layer() const {
  return style_state_.base.layer;
}

float SliderElement::clamped_value(float value) const {
  if (max_ <= min_) {
    return min_;
  }
  float next = std::clamp(value, min_, max_);
  if (step_ > 0.0F) {
    next = min_ + std::round((next - min_) / step_) * step_;
    next = std::clamp(next, min_, max_);
  }
  return next;
}

float SliderElement::value_for_point(Point point) const {
  const Rect track = track_rect();
  if (track.size.width <= 0.0F) {
    return value_;
  }
  const float progress =
      std::clamp((point.x - track.origin.x) / track.size.width, 0.0F, 1.0F);
  return clamped_value(min_ + progress * (max_ - min_));
}

} // namespace cgpui
