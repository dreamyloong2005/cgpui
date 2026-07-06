#include "style_cascade_overlays.hpp"

namespace {

[[nodiscard]] bool same_size(cgpui::Size lhs, cgpui::Size rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

[[nodiscard]] bool same_edges(cgpui::EdgeSizes lhs, cgpui::EdgeSizes rhs) {
  return lhs.top == rhs.top && lhs.right == rhs.right &&
         lhs.bottom == rhs.bottom && lhs.left == rhs.left;
}

[[nodiscard]] bool same_radii(
    cgpui::BorderRadii lhs,
    cgpui::BorderRadii rhs) {
  return lhs.top_left == rhs.top_left && lhs.top_right == rhs.top_right &&
         lhs.bottom_right == rhs.bottom_right &&
         lhs.bottom_left == rhs.bottom_left;
}

[[nodiscard]] bool same_percentage_size(
    const cgpui::PercentageSize& lhs,
    const cgpui::PercentageSize& rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

} // namespace

namespace cgpui::style_cascade_detail {

StyleOverlay style_base_overlay(const Style& style) {
  const Style defaults;
  StyleOverlay overlay;
  overlay.background_color = style.background_color;
  overlay.foreground_color = style.foreground_color;
  overlay.border_color = style.border_color;
  overlay.box_shadow = style.box_shadow;
  overlay.clip_rect = style.clip_rect;
  if (!same_size(style.preferred_size, defaults.preferred_size)) {
    overlay.preferred_size = style.preferred_size;
  }
  if (!same_size(style.min_size, defaults.min_size)) {
    overlay.min_size = style.min_size;
  }
  if (!same_size(style.max_size, defaults.max_size)) {
    overlay.max_size = style.max_size;
  }
  if (!same_percentage_size(
          style.percentage_size,
          defaults.percentage_size)) {
    overlay.percentage_size = style.percentage_size;
  }
  if (!same_edges(style.padding, defaults.padding)) {
    overlay.padding = style.padding;
  }
  if (!same_edges(style.margin, defaults.margin)) {
    overlay.margin = style.margin;
  }
  if (!same_edges(style.border_width, defaults.border_width)) {
    overlay.border_width = style.border_width;
  }
  if (!same_radii(style.border_radius, defaults.border_radius)) {
    overlay.border_radius = style.border_radius;
  }
  if (style.overflow != defaults.overflow) {
    overlay.overflow = style.overflow;
  }
  if (style.z_index != defaults.z_index) {
    overlay.z_index = style.z_index;
  }
  if (style.layer != defaults.layer) {
    overlay.layer = style.layer;
  }
  if (style.gap != defaults.gap) {
    overlay.gap = style.gap;
  }
  if (style.align_items != defaults.align_items) {
    overlay.align_items = style.align_items;
  }
  if (style.justify_content != defaults.justify_content) {
    overlay.justify_content = style.justify_content;
  }
  if (style.flex_grow != defaults.flex_grow) {
    overlay.flex_grow = style.flex_grow;
  }
  if (style.flex_shrink != defaults.flex_shrink) {
    overlay.flex_shrink = style.flex_shrink;
  }
  if (style.position != defaults.position) {
    overlay.position = style.position;
  }
  if (!same_edges(style.inset, defaults.inset)) {
    overlay.inset = style.inset;
  }
  if (style.font != defaults.font) {
    overlay.font = style.font;
  }
  if (style.font_size != defaults.font_size) {
    overlay.font_size = style.font_size;
  }
  if (style.opacity != defaults.opacity) {
    overlay.opacity = style.opacity;
  }
  if (style.transform != defaults.transform) {
    overlay.transform = style.transform;
  }
  overlay.tokens = style.tokens;
  return overlay;
}

Style apply_style_state(
    Style style,
    const StyleState& state,
    StyleStateFlags flags) {
  style = apply_style_overlay(style, style_base_overlay(state.base));
  if (flags.hovered) {
    style = apply_style_overlay(style, state.hover);
  }
  if (flags.focused) {
    style = apply_style_overlay(style, state.focus);
  }
  if (flags.active) {
    style = apply_style_overlay(style, state.active);
  }
  if (flags.disabled) {
    style = apply_style_overlay(style, state.disabled);
  }
  return style;
}

} // namespace cgpui::style_cascade_detail
