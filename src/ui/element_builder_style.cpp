#include "cgpui/ui/element_builder_core.hpp"

#include <string>
#include <utility>

namespace cgpui {

ElementBuilder ElementBuilder::style(Style style) && {
  style_state_.base = std::move(style);
  return std::move(*this);
}

ElementBuilder ElementBuilder::hover_style(StyleOverlay overlay) && {
  style_state_.hover = std::move(overlay);
  return std::move(*this);
}

ElementBuilder ElementBuilder::focus_style(StyleOverlay overlay) && {
  style_state_.focus = std::move(overlay);
  return std::move(*this);
}

ElementBuilder ElementBuilder::disabled_style(StyleOverlay overlay) && {
  style_state_.disabled = std::move(overlay);
  return std::move(*this);
}

ElementBuilder ElementBuilder::class_name(StyleClassId id) && {
  style_classes_.add(std::move(id));
  return std::move(*this);
}

ElementBuilder ElementBuilder::class_name(std::string_view value) && {
  return std::move(*this).class_name(style_class(value));
}

ElementBuilder ElementBuilder::inline_style(StyleOverlay style) && {
  inline_style_ = std::move(style);
  return std::move(*this);
}

ElementBuilder ElementBuilder::key(ElementKey key) && {
  key_ = std::move(key);
  return std::move(*this);
}

ElementBuilder ElementBuilder::key(std::string_view value) && {
  key_ = ElementKey{.value = std::string(value)};
  return std::move(*this);
}

ElementBuilder ElementBuilder::background(Color color) && {
  style_state_.base = style_state_.base.with_background_color(color);
  return std::move(*this);
}

ElementBuilder ElementBuilder::bg(Color color) && {
  return std::move(*this).background(color);
}

ElementBuilder ElementBuilder::foreground(Color color) && {
  style_state_.base = style_state_.base.with_foreground_color(color);
  return std::move(*this);
}

ElementBuilder ElementBuilder::text_color(Color color) && {
  return std::move(*this).foreground(color);
}

ElementBuilder ElementBuilder::font(FontDescriptor descriptor) && {
  style_state_.base = style_state_.base.with_font(std::move(descriptor));
  return std::move(*this);
}

ElementBuilder ElementBuilder::font_size(float value) && {
  style_state_.base = style_state_.base.with_font_size(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::border_color(Color color) && {
  style_state_.base = style_state_.base.with_border_color(color);
  return std::move(*this);
}

ElementBuilder ElementBuilder::border_radius(BorderRadii radius) && {
  style_state_.base = style_state_.base.with_border_radius(radius);
  return std::move(*this);
}

ElementBuilder ElementBuilder::rounded(float radius) && {
  return std::move(*this).border_radius(BorderRadii::all(radius));
}

} // namespace cgpui
