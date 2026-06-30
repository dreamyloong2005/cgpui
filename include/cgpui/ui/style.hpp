#pragma once

#include "cgpui/core/geometry.hpp"

#include <optional>

namespace cgpui {

enum class Overflow {
  visible,
  hidden,
};

enum class AlignItems {
  start,
  center,
  end,
};

enum class JustifyContent {
  start,
  center,
  end,
  space_between,
};

[[nodiscard]] constexpr float px(float value) {
  return value;
}

[[nodiscard]] constexpr Color rgb(int red, int green, int blue) {
  return Color{
      .r = static_cast<float>(red) / 255.0F,
      .g = static_cast<float>(green) / 255.0F,
      .b = static_cast<float>(blue) / 255.0F,
      .a = 1.0F,
  };
}

[[nodiscard]] constexpr Color rgba(
    int red,
    int green,
    int blue,
    float alpha) {
  return Color{
      .r = static_cast<float>(red) / 255.0F,
      .g = static_cast<float>(green) / 255.0F,
      .b = static_cast<float>(blue) / 255.0F,
      .a = alpha,
  };
}

struct EdgeSizes {
  float top = 0.0F;
  float right = 0.0F;
  float bottom = 0.0F;
  float left = 0.0F;

  [[nodiscard]] static constexpr EdgeSizes all(float value) {
    return EdgeSizes{
        .top = value,
        .right = value,
        .bottom = value,
        .left = value,
    };
  }

  [[nodiscard]] static constexpr EdgeSizes axes(
      float horizontal,
      float vertical) {
    return EdgeSizes{
        .top = vertical,
        .right = horizontal,
        .bottom = vertical,
        .left = horizontal,
    };
  }

  [[nodiscard]] static constexpr EdgeSizes trbl(
      float top,
      float right,
      float bottom,
      float left) {
    return EdgeSizes{
        .top = top,
        .right = right,
        .bottom = bottom,
        .left = left,
    };
  }
};

[[nodiscard]] constexpr EdgeSizes edges(float value) {
  return EdgeSizes::all(value);
}

[[nodiscard]] constexpr EdgeSizes edges(float horizontal, float vertical) {
  return EdgeSizes::axes(horizontal, vertical);
}

[[nodiscard]] constexpr EdgeSizes edges(
    float top,
    float right,
    float bottom,
    float left) {
  return EdgeSizes::trbl(top, right, bottom, left);
}

struct BorderRadii {
  float top_left = 0.0F;
  float top_right = 0.0F;
  float bottom_right = 0.0F;
  float bottom_left = 0.0F;

  [[nodiscard]] static constexpr BorderRadii all(float value) {
    return BorderRadii{
        .top_left = value,
        .top_right = value,
        .bottom_right = value,
        .bottom_left = value,
    };
  }

  [[nodiscard]] static constexpr BorderRadii corners(
      float top_left,
      float top_right,
      float bottom_right,
      float bottom_left) {
    return BorderRadii{
        .top_left = top_left,
        .top_right = top_right,
        .bottom_right = bottom_right,
        .bottom_left = bottom_left,
    };
  }
};

struct Style {
  std::optional<Color> background_color;
  std::optional<Color> foreground_color;
  std::optional<Color> border_color;
  std::optional<Rect> clip_rect;
  Size preferred_size;
  EdgeSizes padding;
  EdgeSizes margin;
  EdgeSizes border_width;
  BorderRadii border_radius;
  Overflow overflow = Overflow::visible;
  int z_index = 0;
  float gap = 0.0F;
  AlignItems align_items = AlignItems::start;
  JustifyContent justify_content = JustifyContent::start;

  [[nodiscard]] constexpr Style with_background_color(Color color) const {
    Style style = *this;
    style.background_color = color;
    return style;
  }

  [[nodiscard]] constexpr Style with_foreground_color(Color color) const {
    Style style = *this;
    style.foreground_color = color;
    return style;
  }

  [[nodiscard]] constexpr Style with_preferred_size(Size size) const {
    Style style = *this;
    style.preferred_size = size;
    return style;
  }

  [[nodiscard]] constexpr Style with_padding(EdgeSizes edges) const {
    Style style = *this;
    style.padding = edges;
    return style;
  }

  [[nodiscard]] constexpr Style with_margin(EdgeSizes edges) const {
    Style style = *this;
    style.margin = edges;
    return style;
  }

  [[nodiscard]] constexpr Style with_border_width(EdgeSizes edges) const {
    Style style = *this;
    style.border_width = edges;
    return style;
  }

  [[nodiscard]] constexpr Style with_border_color(Color color) const {
    Style style = *this;
    style.border_color = color;
    return style;
  }

  [[nodiscard]] constexpr Style with_border_radius(BorderRadii radius) const {
    Style style = *this;
    style.border_radius = radius;
    return style;
  }

  [[nodiscard]] constexpr Style with_overflow(Overflow value) const {
    Style style = *this;
    style.overflow = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_z_index(int value) const {
    Style style = *this;
    style.z_index = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_gap(float value) const {
    Style style = *this;
    style.gap = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_align_items(AlignItems value) const {
    Style style = *this;
    style.align_items = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_justify_content(
      JustifyContent value) const {
    Style style = *this;
    style.justify_content = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_clip_rect(Rect rect) const {
    Style style = *this;
    style.clip_rect = rect;
    return style;
  }
};

struct StyleOverlay {
  std::optional<Color> background_color;
  std::optional<Color> foreground_color;
  std::optional<Color> border_color;
  std::optional<Rect> clip_rect;
  std::optional<Size> preferred_size;
  std::optional<EdgeSizes> padding;
  std::optional<EdgeSizes> margin;
  std::optional<EdgeSizes> border_width;
  std::optional<BorderRadii> border_radius;
  std::optional<Overflow> overflow;
  std::optional<int> z_index;
  std::optional<float> gap;
  std::optional<AlignItems> align_items;
  std::optional<JustifyContent> justify_content;

  [[nodiscard]] constexpr StyleOverlay with_background_color(
      Color color) const {
    StyleOverlay overlay = *this;
    overlay.background_color = color;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_foreground_color(
      Color color) const {
    StyleOverlay overlay = *this;
    overlay.foreground_color = color;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_preferred_size(Size size) const {
    StyleOverlay overlay = *this;
    overlay.preferred_size = size;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_padding(EdgeSizes edges) const {
    StyleOverlay overlay = *this;
    overlay.padding = edges;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_margin(EdgeSizes edges) const {
    StyleOverlay overlay = *this;
    overlay.margin = edges;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_border_width(
      EdgeSizes edges) const {
    StyleOverlay overlay = *this;
    overlay.border_width = edges;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_border_color(Color color) const {
    StyleOverlay overlay = *this;
    overlay.border_color = color;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_border_radius(
      BorderRadii radius) const {
    StyleOverlay overlay = *this;
    overlay.border_radius = radius;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_overflow(Overflow value) const {
    StyleOverlay overlay = *this;
    overlay.overflow = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_z_index(int value) const {
    StyleOverlay overlay = *this;
    overlay.z_index = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_gap(float value) const {
    StyleOverlay overlay = *this;
    overlay.gap = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_align_items(
      AlignItems value) const {
    StyleOverlay overlay = *this;
    overlay.align_items = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_justify_content(
      JustifyContent value) const {
    StyleOverlay overlay = *this;
    overlay.justify_content = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_clip_rect(Rect rect) const {
    StyleOverlay overlay = *this;
    overlay.clip_rect = rect;
    return overlay;
  }
};

struct StyleState {
  Style base;
  StyleOverlay hover;
  StyleOverlay focus;
  StyleOverlay disabled;
};

struct StyleStateFlags {
  bool hovered = false;
  bool focused = false;
  bool disabled = false;
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
  if (overlay.clip_rect.has_value()) {
    style.clip_rect = overlay.clip_rect;
  }
  if (overlay.preferred_size.has_value()) {
    style.preferred_size = *overlay.preferred_size;
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
  if (overlay.gap.has_value()) {
    style.gap = *overlay.gap;
  }
  if (overlay.align_items.has_value()) {
    style.align_items = *overlay.align_items;
  }
  if (overlay.justify_content.has_value()) {
    style.justify_content = *overlay.justify_content;
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
  if (flags.disabled) {
    style = apply_style_overlay(style, state.disabled);
  }
  return style;
}

} // namespace cgpui
