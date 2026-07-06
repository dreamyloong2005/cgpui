#include "cgpui/ui/button_builder.hpp"

#include <utility>

namespace cgpui {

ButtonBuilder::ButtonBuilder(std::string action_name)
    : action_name_(std::move(action_name)) {}

ButtonBuilder ButtonBuilder::style(Style style) && {
  style_state_.base = std::move(style);
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::hover_style(StyleOverlay overlay) && {
  style_state_.hover = std::move(overlay);
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::focus_style(StyleOverlay overlay) && {
  style_state_.focus = std::move(overlay);
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::active_style(StyleOverlay overlay) && {
  style_state_.active = std::move(overlay);
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::disabled_style(StyleOverlay overlay) && {
  style_state_.disabled = std::move(overlay);
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::child(std::unique_ptr<Element> child) && {
  child_ = std::move(child);
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::child(ElementBuilder child) && {
  return std::move(*this).child(into_element(std::move(child)));
}

ButtonBuilder ButtonBuilder::on_click(ClickHandler handler) && {
  click_handler_ = std::move(handler);
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::enabled(bool value) && {
  enabled_ = value;
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::disabled() && {
  enabled_ = false;
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::key(ElementKey key) && {
  key_ = std::move(key);
  return std::move(*this);
}

ButtonBuilder ButtonBuilder::key(std::string_view value) && {
  key_ = ElementKey{.value = std::string(value)};
  return std::move(*this);
}

AnyElement ButtonBuilder::build() && {
  auto element = std::make_unique<ButtonElement>(
      std::move(action_name_),
      style_state_,
      std::move(click_handler_),
      std::move(child_));
  element->set_enabled(enabled_);
  element->set_key(key_);
  element->set_flex_grow(style_state_.base.flex_grow);
  element->set_flex_shrink(style_state_.base.flex_shrink);
  element->set_position(style_state_.base.position);
  element->set_inset(style_state_.base.inset);
  element->set_z_index(style_state_.base.z_index);
  element->set_layer(style_state_.base.layer);
  return element;
}

ButtonBuilder button(std::string_view action_name) {
  return ButtonBuilder(std::string(action_name));
}

} // namespace cgpui
