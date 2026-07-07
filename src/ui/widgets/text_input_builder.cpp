#include "cgpui/ui/text_input_builder.hpp"

#include <string>
#include <utility>

namespace cgpui {

TextInputBuilder::TextInputBuilder(TextModel& model) : model_(&model) {}

TextInputBuilder TextInputBuilder::style(Style style) && {
  style_ = std::move(style);
  return std::move(*this);
}

TextInputBuilder TextInputBuilder::foreground(Color color) && {
  style_ = style_.with_foreground_color(color);
  return std::move(*this);
}

TextInputBuilder TextInputBuilder::font(FontDescriptor descriptor) && {
  style_ = style_.with_font(std::move(descriptor));
  return std::move(*this);
}

TextInputBuilder TextInputBuilder::font_size(float value) && {
  style_ = style_.with_font_size(value);
  return std::move(*this);
}

TextInputBuilder TextInputBuilder::key(ElementKey key) && {
  key_ = std::move(key);
  return std::move(*this);
}

TextInputBuilder TextInputBuilder::key(std::string_view value) && {
  key_ = ElementKey{.value = std::string(value)};
  return std::move(*this);
}

TextInputBuilder TextInputBuilder::enabled(bool value) && {
  enabled_ = value;
  return std::move(*this);
}

TextInputBuilder TextInputBuilder::disabled() && {
  enabled_ = false;
  return std::move(*this);
}

AnyElement TextInputBuilder::build() && {
  auto element = std::make_unique<TextInputElement>(*model_, style_);
  element->set_enabled(enabled_);
  element->set_key(key_);
  element->set_flex_grow(element->style().flex_grow);
  element->set_flex_shrink(element->style().flex_shrink);
  element->set_position(element->style().position);
  element->set_inset(element->style().inset);
  element->set_z_index(element->style().z_index);
  element->set_layer(element->style().layer);
  return element;
}

TextInputBuilder text_input(TextModel& model) {
  return TextInputBuilder(model);
}

} // namespace cgpui
