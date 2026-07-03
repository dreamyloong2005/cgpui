#include "cgpui/ui/scrollable_list_builder.hpp"

#include <string>
#include <utility>

namespace cgpui {

ScrollableListBuilder::ScrollableListBuilder(ScrollState& state)
    : state_(&state) {}

ScrollableListBuilder ScrollableListBuilder::style(Style style) && {
  style_ = std::move(style);
  return std::move(*this);
}

ScrollableListBuilder ScrollableListBuilder::size(Size size) && {
  style_ = style_.with_preferred_size(size);
  return std::move(*this);
}

ScrollableListBuilder ScrollableListBuilder::size(
    float width,
    float height) && {
  return std::move(*this).size(Size{.width = width, .height = height});
}

ScrollableListBuilder ScrollableListBuilder::gap(float value) && {
  gap_ = value;
  return std::move(*this);
}

ScrollableListBuilder ScrollableListBuilder::key(ElementKey key) && {
  key_ = std::move(key);
  return std::move(*this);
}

ScrollableListBuilder ScrollableListBuilder::key(std::string_view value) && {
  key_ = ElementKey{.value = std::string(value)};
  return std::move(*this);
}

ScrollableListBuilder ScrollableListBuilder::enabled(bool value) && {
  enabled_ = value;
  return std::move(*this);
}

ScrollableListBuilder ScrollableListBuilder::disabled() && {
  enabled_ = false;
  return std::move(*this);
}

ScrollableListBuilder ScrollableListBuilder::item(
    ElementKey key,
    AnyElement child) && {
  if (child) {
    child->set_key(std::move(key));
    items_.push_back(std::move(child));
  }
  return std::move(*this);
}

ScrollableListBuilder ScrollableListBuilder::item(
    std::string_view key,
    AnyElement child) && {
  return std::move(*this).item(
      ElementKey{.value = std::string(key)},
      std::move(child));
}

ScrollableListBuilder ScrollableListBuilder::item(
    std::string_view key,
    ElementBuilder child) && {
  return std::move(*this).item(key, into_element(std::move(child)));
}

AnyElement ScrollableListBuilder::build() && {
  auto element = std::make_unique<ScrollableListElement>(
      *state_,
      style_,
      std::move(items_),
      gap_);
  element->set_enabled(enabled_);
  element->set_key(key_);
  element->set_flex_grow(style_.flex_grow);
  element->set_flex_shrink(style_.flex_shrink);
  element->set_position(style_.position);
  element->set_inset(style_.inset);
  element->set_z_index(style_.z_index);
  element->set_layer(style_.layer);
  return element;
}

ScrollableListBuilder scrollable_list(ScrollState& state) {
  return ScrollableListBuilder(state);
}

AnyElement scroll(ScrollState& state, AnyElement child) {
  return std::make_unique<ScrollElement>(state, std::move(child));
}

AnyElement scroll(ScrollState& state, ElementBuilder child) {
  return scroll(state, into_element(std::move(child)));
}

} // namespace cgpui
