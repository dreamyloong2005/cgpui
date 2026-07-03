#include "cgpui/ui/label_builder.hpp"

#include <utility>

namespace cgpui {

LabelBuilder::LabelBuilder(std::string text) : text_(std::move(text)) {}

LabelBuilder LabelBuilder::style(Style style) && {
  style_ = std::move(style);
  return std::move(*this);
}

LabelBuilder LabelBuilder::foreground(Color color) && {
  style_ = style_.with_foreground_color(color);
  return std::move(*this);
}

LabelBuilder LabelBuilder::font(FontDescriptor descriptor) && {
  style_ = style_.with_font(std::move(descriptor));
  return std::move(*this);
}

LabelBuilder LabelBuilder::font_size(float value) && {
  style_ = style_.with_font_size(value);
  return std::move(*this);
}

LabelBuilder LabelBuilder::key(ElementKey key) && {
  key_ = std::move(key);
  return std::move(*this);
}

LabelBuilder LabelBuilder::key(std::string_view value) && {
  key_ = ElementKey{.value = std::string(value)};
  return std::move(*this);
}

LabelBuilder LabelBuilder::enabled(bool value) && {
  enabled_ = value;
  return std::move(*this);
}

LabelBuilder LabelBuilder::disabled() && {
  enabled_ = false;
  return std::move(*this);
}

AnyElement LabelBuilder::build() && {
  auto element =
      std::make_unique<LabelElement>(std::move(text_), std::move(style_));
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

LabelBuilder label(std::string_view text) {
  return LabelBuilder(std::string(text));
}

} // namespace cgpui
