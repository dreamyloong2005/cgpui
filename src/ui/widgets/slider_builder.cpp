#include "cgpui/ui/slider_builder.hpp"

#include <utility>

namespace cgpui {

SliderBuilder::SliderBuilder(std::string action_name)
    : action_name_(std::move(action_name)) {}

SliderBuilder SliderBuilder::style(Style style) && {
  style_state_.base = std::move(style);
  return std::move(*this);
}

SliderBuilder SliderBuilder::hover_style(StyleOverlay overlay) && {
  style_state_.hover = std::move(overlay);
  return std::move(*this);
}

SliderBuilder SliderBuilder::focus_style(StyleOverlay overlay) && {
  style_state_.focus = std::move(overlay);
  return std::move(*this);
}

SliderBuilder SliderBuilder::active_style(StyleOverlay overlay) && {
  style_state_.active = std::move(overlay);
  return std::move(*this);
}

SliderBuilder SliderBuilder::disabled_style(StyleOverlay overlay) && {
  style_state_.disabled = std::move(overlay);
  return std::move(*this);
}

SliderBuilder SliderBuilder::range(float min, float max) && {
  min_ = min;
  max_ = max;
  return std::move(*this);
}

SliderBuilder SliderBuilder::min(float value) && {
  min_ = value;
  return std::move(*this);
}

SliderBuilder SliderBuilder::max(float value) && {
  max_ = value;
  return std::move(*this);
}

SliderBuilder SliderBuilder::value(float value) && {
  value_ = value;
  return std::move(*this);
}

SliderBuilder SliderBuilder::step(float value) && {
  step_ = value;
  return std::move(*this);
}

SliderBuilder SliderBuilder::on_change(SliderChangeHandler handler) && {
  change_handler_ = std::move(handler);
  return std::move(*this);
}

SliderBuilder SliderBuilder::enabled(bool value) && {
  enabled_ = value;
  return std::move(*this);
}

SliderBuilder SliderBuilder::disabled() && {
  enabled_ = false;
  return std::move(*this);
}

SliderBuilder SliderBuilder::key(ElementKey key) && {
  key_ = std::move(key);
  return std::move(*this);
}

SliderBuilder SliderBuilder::key(std::string_view value) && {
  key_ = ElementKey{.value = std::string(value)};
  return std::move(*this);
}

AnyElement SliderBuilder::build() && {
  auto element = std::make_unique<SliderElement>(
      std::move(action_name_),
      style_state_,
      min_,
      max_,
      value_,
      step_,
      std::move(change_handler_));
  element->set_enabled(enabled_);
  element->set_key(key_);
  element->set_flex_grow(element->style_state().base.flex_grow);
  element->set_flex_shrink(element->style_state().base.flex_shrink);
  element->set_position(element->style_state().base.position);
  element->set_inset(element->style_state().base.inset);
  element->set_z_index(element->style_state().base.z_index);
  element->set_layer(element->style_state().base.layer);
  return element;
}

SliderBuilder slider(std::string_view action_name) {
  return SliderBuilder(std::string(action_name));
}

} // namespace cgpui
