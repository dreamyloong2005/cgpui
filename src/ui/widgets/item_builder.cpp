#include "cgpui/ui/item_builder.hpp"

#include "cgpui/ui/label_builder.hpp"

#include <utility>

namespace cgpui {

ItemBuilder::ItemBuilder(ItemKind kind, std::string action_name)
    : kind_(kind),
      action_name_(std::move(action_name)) {}

ItemBuilder ItemBuilder::style(Style style) && {
  style_state_.base = std::move(style);
  return std::move(*this);
}

ItemBuilder ItemBuilder::hover_style(StyleOverlay overlay) && {
  style_state_.hover = std::move(overlay);
  return std::move(*this);
}

ItemBuilder ItemBuilder::focus_style(StyleOverlay overlay) && {
  style_state_.focus = std::move(overlay);
  return std::move(*this);
}

ItemBuilder ItemBuilder::active_style(StyleOverlay overlay) && {
  style_state_.active = std::move(overlay);
  return std::move(*this);
}

ItemBuilder ItemBuilder::disabled_style(StyleOverlay overlay) && {
  style_state_.disabled = std::move(overlay);
  return std::move(*this);
}

ItemBuilder ItemBuilder::label(std::string_view text) && {
  return std::move(*this).child(cgpui::label(text).build());
}

ItemBuilder ItemBuilder::child(std::unique_ptr<Element> child) && {
  child_ = std::move(child);
  return std::move(*this);
}

ItemBuilder ItemBuilder::child(ElementBuilder child) && {
  return std::move(*this).child(into_element(std::move(child)));
}

ItemBuilder ItemBuilder::selected(bool value) && {
  selected_ = value;
  return std::move(*this);
}

ItemBuilder ItemBuilder::on_click(ClickHandler handler) && {
  click_handler_ = std::move(handler);
  return std::move(*this);
}

ItemBuilder ItemBuilder::enabled(bool value) && {
  enabled_ = value;
  return std::move(*this);
}

ItemBuilder ItemBuilder::disabled() && {
  enabled_ = false;
  return std::move(*this);
}

ItemBuilder ItemBuilder::key(ElementKey key) && {
  key_ = std::move(key);
  return std::move(*this);
}

ItemBuilder ItemBuilder::key(std::string_view value) && {
  key_ = ElementKey{.value = std::string(value)};
  return std::move(*this);
}

AnyElement ItemBuilder::build() && {
  auto element = std::make_unique<ItemElement>(
      kind_,
      std::move(action_name_),
      style_state_,
      selected_,
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

ItemBuilder list_item(std::string_view action_name) {
  return ItemBuilder(ItemKind::list_item, std::string(action_name));
}

ItemBuilder menu_item(std::string_view action_name) {
  return ItemBuilder(ItemKind::menu_item, std::string(action_name));
}

} // namespace cgpui
