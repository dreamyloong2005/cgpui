#pragma once

#include "cgpui/ui/style_core.hpp"
#include "cgpui/ui/style_tokens.hpp"

#include <string>
#include <unordered_map>
#include <utility>

namespace cgpui {

class StyleCascade {
 public:
  StyleCascade& set_class_style(StyleClassId id, StyleState style);

  [[nodiscard]] const StyleState* class_style(
      const StyleClassId& id) const;

 private:
  std::unordered_map<std::string, StyleState> class_styles_;
};

inline StyleCascade& StyleCascade::set_class_style(
    StyleClassId id,
    StyleState style) {
  class_styles_[std::move(id.value)] = std::move(style);
  return *this;
}

inline const StyleState* StyleCascade::class_style(
    const StyleClassId& id) const {
  const auto entry = class_styles_.find(id.value);
  if (entry == class_styles_.end()) {
    return nullptr;
  }
  return &entry->second;
}

namespace detail {

[[nodiscard]] inline bool same_size(Size lhs, Size rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

[[nodiscard]] inline bool same_edges(EdgeSizes lhs, EdgeSizes rhs) {
  return lhs.top == rhs.top && lhs.right == rhs.right &&
         lhs.bottom == rhs.bottom && lhs.left == rhs.left;
}

[[nodiscard]] inline bool same_radii(BorderRadii lhs, BorderRadii rhs) {
  return lhs.top_left == rhs.top_left && lhs.top_right == rhs.top_right &&
         lhs.bottom_right == rhs.bottom_right &&
         lhs.bottom_left == rhs.bottom_left;
}

[[nodiscard]] inline bool same_percentage_size(
    const PercentageSize& lhs,
    const PercentageSize& rhs) {
  return lhs.width == rhs.width && lhs.height == rhs.height;
}

[[nodiscard]] inline StyleOverlay style_base_overlay(const Style& style) {
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
  return overlay;
}

} // namespace detail

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

[[nodiscard]] inline Style resolved_style(
    const StyleCascade& cascade,
    const StyleState& local,
    const StyleClasses& classes,
    const StyleOverlay& inline_style,
    StyleStateFlags flags) {
  Style style = local.base;
  for (const StyleClassId& class_id : classes.ids()) {
    const StyleState* class_style = cascade.class_style(class_id);
    if (class_style == nullptr) {
      continue;
    }
    style = apply_style_overlay(
        style,
        detail::style_base_overlay(class_style->base));
    if (flags.hovered) {
      style = apply_style_overlay(style, class_style->hover);
    }
    if (flags.focused) {
      style = apply_style_overlay(style, class_style->focus);
    }
    if (flags.active) {
      style = apply_style_overlay(style, class_style->active);
    }
    if (flags.disabled) {
      style = apply_style_overlay(style, class_style->disabled);
    }
  }

  style = resolved_style(StyleState{
                             .base = style,
                             .hover = local.hover,
                             .focus = local.focus,
                             .active = local.active,
                             .disabled = local.disabled,
                         },
                         flags);
  return apply_style_overlay(style, inline_style);
}

} // namespace cgpui
