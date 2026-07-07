#include "cgpui/ui/toggle_builder.hpp"

#include "cgpui/ui/label_builder.hpp"

#include <utility>

namespace cgpui {

ToggleBuilder::ToggleBuilder(ToggleControlKind kind, std::string action_name)
    : kind_(kind),
      action_name_(std::move(action_name)) {}

ToggleBuilder ToggleBuilder::style(Style style) && {
  style_state_.base = std::move(style);
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::hover_style(StyleOverlay overlay) && {
  style_state_.hover = std::move(overlay);
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::focus_style(StyleOverlay overlay) && {
  style_state_.focus = std::move(overlay);
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::active_style(StyleOverlay overlay) && {
  style_state_.active = std::move(overlay);
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::disabled_style(StyleOverlay overlay) && {
  style_state_.disabled = std::move(overlay);
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::label(std::string_view text) && {
  return std::move(*this).child(cgpui::label(text).build());
}

ToggleBuilder ToggleBuilder::child(std::unique_ptr<Element> child) && {
  child_ = std::move(child);
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::child(ElementBuilder child) && {
  return std::move(*this).child(into_element(std::move(child)));
}

ToggleBuilder ToggleBuilder::checked(bool value) && {
  checked_ = value;
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::selected(bool value) && {
  return std::move(*this).checked(value);
}

ToggleBuilder ToggleBuilder::on(bool value) && {
  return std::move(*this).checked(value);
}

ToggleBuilder ToggleBuilder::on_click(ClickHandler handler) && {
  click_handler_ = std::move(handler);
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::enabled(bool value) && {
  enabled_ = value;
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::disabled() && {
  enabled_ = false;
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::key(ElementKey key) && {
  key_ = std::move(key);
  return std::move(*this);
}

ToggleBuilder ToggleBuilder::key(std::string_view value) && {
  key_ = ElementKey{.value = std::string(value)};
  return std::move(*this);
}

AnyElement ToggleBuilder::build() && {
  auto element = std::make_unique<ToggleControlElement>(
      kind_,
      std::move(action_name_),
      style_state_,
      checked_,
      std::move(click_handler_),
      std::move(child_));
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

ToggleBuilder checkbox(std::string_view action_name) {
  return ToggleBuilder(ToggleControlKind::checkbox, std::string(action_name));
}

ToggleBuilder radio(std::string_view action_name) {
  return ToggleBuilder(ToggleControlKind::radio, std::string(action_name));
}

ToggleBuilder toggle_switch(std::string_view action_name) {
  return ToggleBuilder(
      ToggleControlKind::toggle_switch,
      std::string(action_name));
}

} // namespace cgpui
