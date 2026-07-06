#pragma once

#include "cgpui/ui/style_core.hpp"
#include "cgpui/ui/style_tokens.hpp"

#include <string>
#include <unordered_map>

namespace cgpui {

struct StyleClassRule {
  StyleClasses reused_classes;
  StyleState style;
};

class StyleCascade {
 public:
  StyleCascade& set_class_style(StyleClassId id, StyleState style);
  StyleCascade& set_class_rule(StyleClassId id, StyleClassRule rule);

  [[nodiscard]] const StyleState* class_style(
      const StyleClassId& id) const;
  [[nodiscard]] const StyleClassRule* class_rule(
      const StyleClassId& id) const;

 private:
  std::unordered_map<std::string, StyleClassRule> class_styles_;
};

[[nodiscard]] constexpr Style apply_style_overlay(
    Style style,
    const StyleOverlay& overlay) {
  if (overlay.background_color.has_value()) {
    style.background_color = overlay.background_color;
  }
  if (overlay.foreground_color.has_value()) {
    style.foreground_color = overlay.foreground_color;
  }
  if (overlay.border_color.has_value()) {
    style.border_color = overlay.border_color;
  }
  if (overlay.box_shadow.has_value()) {
    style.box_shadow = overlay.box_shadow;
  }
  if (overlay.clip_rect.has_value()) {
    style.clip_rect = overlay.clip_rect;
  }
  if (overlay.preferred_size.has_value()) {
    style.preferred_size = *overlay.preferred_size;
  }
  if (overlay.min_size.has_value()) {
    style.min_size = *overlay.min_size;
  }
  if (overlay.max_size.has_value()) {
    style.max_size = *overlay.max_size;
  }
  if (overlay.percentage_size.width.has_value()) {
    style.percentage_size.width = overlay.percentage_size.width;
  }
  if (overlay.percentage_size.height.has_value()) {
    style.percentage_size.height = overlay.percentage_size.height;
  }
  if (overlay.padding.has_value()) {
    style.padding = *overlay.padding;
  }
  if (overlay.margin.has_value()) {
    style.margin = *overlay.margin;
  }
  if (overlay.border_width.has_value()) {
    style.border_width = *overlay.border_width;
  }
  if (overlay.border_radius.has_value()) {
    style.border_radius = *overlay.border_radius;
  }
  if (overlay.overflow.has_value()) {
    style.overflow = *overlay.overflow;
  }
  if (overlay.z_index.has_value()) {
    style.z_index = *overlay.z_index;
  }
  if (overlay.layer.has_value()) {
    style.layer = *overlay.layer;
  }
  if (overlay.gap.has_value()) {
    style.gap = *overlay.gap;
  }
  if (overlay.align_items.has_value()) {
    style.align_items = *overlay.align_items;
  }
  if (overlay.justify_content.has_value()) {
    style.justify_content = *overlay.justify_content;
  }
  if (overlay.flex_grow.has_value()) {
    style.flex_grow = *overlay.flex_grow;
  }
  if (overlay.flex_shrink.has_value()) {
    style.flex_shrink = *overlay.flex_shrink;
  }
  if (overlay.position.has_value()) {
    style.position = *overlay.position;
  }
  if (overlay.inset.has_value()) {
    style.inset = *overlay.inset;
  }
  if (overlay.font.has_value()) {
    style.font = *overlay.font;
  }
  if (overlay.font_size.has_value()) {
    style.font_size = *overlay.font_size;
  }
  if (overlay.opacity.has_value()) {
    style.opacity = *overlay.opacity;
  }
  if (overlay.transform.has_value()) {
    style.transform = *overlay.transform;
  }
  return style;
}

[[nodiscard]] Style apply_style_overlay(
    Style style,
    const StyleOverlay& overlay,
    const Theme& theme);

[[nodiscard]] constexpr Style resolved_style(
    const StyleState& state,
    StyleStateFlags flags) {
  Style style = state.base;
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

[[nodiscard]] Style resolved_style(
    const StyleState& state,
    StyleStateFlags flags,
    const Theme& theme);

[[nodiscard]] Style resolved_style(
    const StyleCascade& cascade,
    const StyleState& local,
    const StyleClasses& classes,
    const StyleOverlay& inline_style,
    StyleStateFlags flags);

[[nodiscard]] Style resolved_style(
    const StyleCascade& cascade,
    const StyleState& local,
    const StyleClasses& classes,
    const StyleOverlay& inline_style,
    StyleStateFlags flags,
    const Theme& theme);

} // namespace cgpui
