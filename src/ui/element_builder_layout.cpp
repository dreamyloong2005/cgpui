#include "cgpui/ui/element_builder_core.hpp"

#include <utility>

namespace cgpui {

ElementBuilder ElementBuilder::size(Size size) && {
  style_state_.base = style_state_.base.with_preferred_size(size);
  if (kind_ == Kind::fixed_size || kind_ == Kind::child_view) {
    size_ = size;
  }
  return std::move(*this);
}

ElementBuilder ElementBuilder::size(float width, float height) && {
  return std::move(*this).size(Size{.width = width, .height = height});
}

ElementBuilder ElementBuilder::padding(EdgeSizes edges) && {
  style_state_.base = style_state_.base.with_padding(edges);
  return std::move(*this);
}

ElementBuilder ElementBuilder::margin(EdgeSizes edges) && {
  style_state_.base = style_state_.base.with_margin(edges);
  return std::move(*this);
}

ElementBuilder ElementBuilder::border_width(EdgeSizes edges) && {
  style_state_.base = style_state_.base.with_border_width(edges);
  return std::move(*this);
}

ElementBuilder ElementBuilder::gap(float value) && {
  style_state_.base = style_state_.base.with_gap(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::align_items(AlignItems value) && {
  style_state_.base = style_state_.base.with_align_items(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::justify_content(JustifyContent value) && {
  style_state_.base = style_state_.base.with_justify_content(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::flex_grow(float value) && {
  style_state_.base = style_state_.base.with_flex_grow(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::flex_shrink(float value) && {
  style_state_.base = style_state_.base.with_flex_shrink(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::layer(int value) && {
  style_state_.base = style_state_.base.with_layer(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::position(Position value) && {
  style_state_.base = style_state_.base.with_position(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::absolute() && {
  return std::move(*this).position(Position::absolute);
}

ElementBuilder ElementBuilder::inset(EdgeSizes edges) && {
  style_state_.base = style_state_.base.with_inset(edges);
  return std::move(*this);
}

} // namespace cgpui
