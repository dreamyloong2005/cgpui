#include "cgpui/ui/style_box.hpp"

#include <utility>

namespace cgpui {

Style Style::with_background_color(Color color) const {
  Style style = *this;
  style.background_color = color;
  return style;
}

Style Style::with_foreground_color(Color color) const {
  Style style = *this;
  style.foreground_color = color;
  return style;
}

Style Style::with_preferred_size(Size size) const {
  Style style = *this;
  style.preferred_size = size;
  return style;
}

Style Style::with_padding(EdgeSizes edges) const {
  Style style = *this;
  style.padding = edges;
  return style;
}

Style Style::with_margin(EdgeSizes edges) const {
  Style style = *this;
  style.margin = edges;
  return style;
}

Style Style::with_border_width(EdgeSizes edges) const {
  Style style = *this;
  style.border_width = edges;
  return style;
}

Style Style::with_border_color(Color color) const {
  Style style = *this;
  style.border_color = color;
  return style;
}

Style Style::with_border_radius(BorderRadii radius) const {
  Style style = *this;
  style.border_radius = radius;
  return style;
}

Style Style::with_box_shadow(BoxShadow shadow) const {
  Style style = *this;
  style.box_shadow = shadow;
  return style;
}

Style Style::with_overflow(Overflow value) const {
  Style style = *this;
  style.overflow = value;
  return style;
}

Style Style::with_z_index(int value) const {
  Style style = *this;
  style.z_index = value;
  return style;
}

Style Style::with_layer(int value) const {
  Style style = *this;
  style.layer = value;
  return style;
}

Style Style::with_gap(float value) const {
  Style style = *this;
  style.gap = value;
  return style;
}

Style Style::with_align_items(AlignItems value) const {
  Style style = *this;
  style.align_items = value;
  return style;
}

Style Style::with_justify_content(JustifyContent value) const {
  Style style = *this;
  style.justify_content = value;
  return style;
}

Style Style::with_flex_grow(float value) const {
  Style style = *this;
  style.flex_grow = value;
  return style;
}

Style Style::with_flex_shrink(float value) const {
  Style style = *this;
  style.flex_shrink = value;
  return style;
}

Style Style::with_position(Position value) const {
  Style style = *this;
  style.position = value;
  return style;
}

Style Style::with_inset(EdgeSizes edges) const {
  Style style = *this;
  style.inset = edges;
  return style;
}

Style Style::with_font(FontDescriptor descriptor) const {
  Style style = *this;
  style.font = std::move(descriptor);
  return style;
}

Style Style::with_font_size(float value) const {
  Style style = *this;
  style.font_size = value;
  return style;
}

Style Style::with_clip_rect(Rect rect) const {
  Style style = *this;
  style.clip_rect = rect;
  return style;
}

Style Style::with_opacity(float value) const {
  Style style = *this;
  style.opacity = value;
  return style;
}

Style Style::with_transform(AffineTransform value) const {
  Style style = *this;
  style.transform = value;
  return style;
}

} // namespace cgpui
