#include "cgpui/ui/style_overlay.hpp"

#include <utility>

namespace cgpui {

StyleOverlay StyleOverlay::with_background_color(Color color) const {
  StyleOverlay overlay = *this;
  overlay.background_color = color;
  return overlay;
}

StyleOverlay StyleOverlay::with_foreground_color(Color color) const {
  StyleOverlay overlay = *this;
  overlay.foreground_color = color;
  return overlay;
}

StyleOverlay StyleOverlay::with_preferred_size(Size size) const {
  StyleOverlay overlay = *this;
  overlay.preferred_size = size;
  return overlay;
}

StyleOverlay StyleOverlay::with_padding(EdgeSizes edges) const {
  StyleOverlay overlay = *this;
  overlay.padding = edges;
  return overlay;
}

StyleOverlay StyleOverlay::with_margin(EdgeSizes edges) const {
  StyleOverlay overlay = *this;
  overlay.margin = edges;
  return overlay;
}

StyleOverlay StyleOverlay::with_border_width(EdgeSizes edges) const {
  StyleOverlay overlay = *this;
  overlay.border_width = edges;
  return overlay;
}

StyleOverlay StyleOverlay::with_border_color(Color color) const {
  StyleOverlay overlay = *this;
  overlay.border_color = color;
  return overlay;
}

StyleOverlay StyleOverlay::with_border_radius(BorderRadii radius) const {
  StyleOverlay overlay = *this;
  overlay.border_radius = radius;
  return overlay;
}

StyleOverlay StyleOverlay::with_box_shadow(BoxShadow shadow) const {
  StyleOverlay overlay = *this;
  overlay.box_shadow = shadow;
  return overlay;
}

StyleOverlay StyleOverlay::with_overflow(Overflow value) const {
  StyleOverlay overlay = *this;
  overlay.overflow = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_z_index(int value) const {
  StyleOverlay overlay = *this;
  overlay.z_index = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_layer(int value) const {
  StyleOverlay overlay = *this;
  overlay.layer = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_gap(float value) const {
  StyleOverlay overlay = *this;
  overlay.gap = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_align_items(AlignItems value) const {
  StyleOverlay overlay = *this;
  overlay.align_items = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_justify_content(JustifyContent value) const {
  StyleOverlay overlay = *this;
  overlay.justify_content = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_flex_grow(float value) const {
  StyleOverlay overlay = *this;
  overlay.flex_grow = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_flex_shrink(float value) const {
  StyleOverlay overlay = *this;
  overlay.flex_shrink = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_position(Position value) const {
  StyleOverlay overlay = *this;
  overlay.position = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_inset(EdgeSizes edges) const {
  StyleOverlay overlay = *this;
  overlay.inset = edges;
  return overlay;
}

StyleOverlay StyleOverlay::with_font(FontDescriptor descriptor) const {
  StyleOverlay overlay = *this;
  overlay.font = std::move(descriptor);
  return overlay;
}

StyleOverlay StyleOverlay::with_font_size(float value) const {
  StyleOverlay overlay = *this;
  overlay.font_size = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_clip_rect(Rect rect) const {
  StyleOverlay overlay = *this;
  overlay.clip_rect = rect;
  return overlay;
}

StyleOverlay StyleOverlay::with_opacity(float value) const {
  StyleOverlay overlay = *this;
  overlay.opacity = value;
  return overlay;
}

StyleOverlay StyleOverlay::with_transform(AffineTransform value) const {
  StyleOverlay overlay = *this;
  overlay.transform = value;
  return overlay;
}

} // namespace cgpui
