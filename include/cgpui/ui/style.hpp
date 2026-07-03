#pragma once

#include "cgpui/core/geometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cgpui {

struct StyleClassId {
  std::string value;

  friend bool operator==(
      const StyleClassId&,
      const StyleClassId&) = default;
};

struct ThemeTokenId {
  std::string value;

  friend bool operator==(
      const ThemeTokenId&,
      const ThemeTokenId&) = default;
};

[[nodiscard]] inline StyleClassId style_class(std::string_view value) {
  return StyleClassId{.value = std::string(value)};
}

[[nodiscard]] inline ThemeTokenId theme_token(std::string_view value) {
  return ThemeTokenId{.value = std::string(value)};
}

class StyleClasses {
 public:
  StyleClasses& add(StyleClassId id) {
    if (!contains(id)) {
      ids_.push_back(std::move(id));
    }
    return *this;
  }

  [[nodiscard]] bool contains(const StyleClassId& id) const {
    return std::find(ids_.begin(), ids_.end(), id) != ids_.end();
  }

  [[nodiscard]] bool empty() const {
    return ids_.empty();
  }

  [[nodiscard]] std::size_t size() const {
    return ids_.size();
  }

  [[nodiscard]] std::span<const StyleClassId> ids() const {
    return ids_;
  }

 private:
  std::vector<StyleClassId> ids_;
};

class Theme {
 public:
  Theme& set_color(ThemeTokenId id, Color color) {
    colors_[std::move(id.value)] = color;
    return *this;
  }

  [[nodiscard]] std::optional<Color> color(const ThemeTokenId& id) const {
    const auto entry = colors_.find(id.value);
    if (entry == colors_.end()) {
      return std::nullopt;
    }
    return entry->second;
  }

  Theme& set_spacing(ThemeTokenId id, float value) {
    spacings_[std::move(id.value)] = value;
    return *this;
  }

  [[nodiscard]] std::optional<float> spacing(const ThemeTokenId& id) const {
    const auto entry = spacings_.find(id.value);
    if (entry == spacings_.end()) {
      return std::nullopt;
    }
    return entry->second;
  }

 private:
  std::unordered_map<std::string, Color> colors_;
  std::unordered_map<std::string, float> spacings_;
};

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

enum class Position {
  relative,
  absolute,
};

enum class AnimationEasing {
  linear,
  ease_in,
  ease_out,
  ease_in_out,
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

[[nodiscard]] inline float clamp_animation_progress(float progress) {
  return std::clamp(progress, 0.0F, 1.0F);
}

[[nodiscard]] inline float ease(AnimationEasing easing, float progress) {
  const float clamped = clamp_animation_progress(progress);
  switch (easing) {
    case AnimationEasing::ease_in:
      return clamped * clamped;
    case AnimationEasing::ease_out:
      return 1.0F - (1.0F - clamped) * (1.0F - clamped);
    case AnimationEasing::ease_in_out:
      if (clamped < 0.5F) {
        return 2.0F * clamped * clamped;
      }
      return 1.0F - 2.0F * (1.0F - clamped) * (1.0F - clamped);
    case AnimationEasing::linear:
    default:
      return clamped;
  }
}

[[nodiscard]] inline float tween(float from, float to, float progress) {
  const double clamped =
      static_cast<double>(clamp_animation_progress(progress));
  return static_cast<float>(
      static_cast<double>(from) +
      (static_cast<double>(to) - static_cast<double>(from)) * clamped);
}

[[nodiscard]] inline float tween_opacity(float from, float to, float progress) {
  const double value = static_cast<double>(tween(from, to, progress));
  return static_cast<float>(std::round(value * 1'000'000.0) / 1'000'000.0);
}

[[nodiscard]] inline Color tween(Color from, Color to, float progress) {
  return Color{
      .r = tween(from.r, to.r, progress),
      .g = tween(from.g, to.g, progress),
      .b = tween(from.b, to.b, progress),
      .a = tween(from.a, to.a, progress),
  };
}

[[nodiscard]] inline AffineTransform tween(
    AffineTransform from,
    AffineTransform to,
    float progress) {
  return AffineTransform{
      .scale_x = tween(from.scale_x, to.scale_x, progress),
      .skew_y = tween(from.skew_y, to.skew_y, progress),
      .skew_x = tween(from.skew_x, to.skew_x, progress),
      .scale_y = tween(from.scale_y, to.scale_y, progress),
      .translate_x = tween(from.translate_x, to.translate_x, progress),
      .translate_y = tween(from.translate_y, to.translate_y, progress),
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

struct FontDescriptor {
  std::string family;

  friend bool operator==(
      const FontDescriptor&,
      const FontDescriptor&) = default;
};

struct Style {
  std::optional<Color> background_color;
  std::optional<Color> foreground_color;
  std::optional<Color> border_color;
  std::optional<Rect> clip_rect;
  FontDescriptor font;
  Size preferred_size;
  EdgeSizes padding;
  EdgeSizes margin;
  EdgeSizes border_width;
  BorderRadii border_radius;
  Overflow overflow = Overflow::visible;
  int z_index = 0;
  int layer = 0;
  float gap = 0.0F;
  AlignItems align_items = AlignItems::start;
  JustifyContent justify_content = JustifyContent::start;
  float flex_grow = 0.0F;
  float flex_shrink = 0.0F;
  Position position = Position::relative;
  EdgeSizes inset;
  float font_size = 16.0F;
  float opacity = 1.0F;
  AffineTransform transform;

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

  [[nodiscard]] constexpr Style with_layer(int value) const {
    Style style = *this;
    style.layer = value;
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

  [[nodiscard]] constexpr Style with_flex_grow(float value) const {
    Style style = *this;
    style.flex_grow = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_flex_shrink(float value) const {
    Style style = *this;
    style.flex_shrink = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_position(Position value) const {
    Style style = *this;
    style.position = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_inset(EdgeSizes edges) const {
    Style style = *this;
    style.inset = edges;
    return style;
  }

  [[nodiscard]] Style with_font(FontDescriptor descriptor) const {
    Style style = *this;
    style.font = descriptor;
    return style;
  }

  [[nodiscard]] constexpr Style with_font_size(float value) const {
    Style style = *this;
    style.font_size = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_clip_rect(Rect rect) const {
    Style style = *this;
    style.clip_rect = rect;
    return style;
  }

  [[nodiscard]] constexpr Style with_opacity(float value) const {
    Style style = *this;
    style.opacity = value;
    return style;
  }

  [[nodiscard]] constexpr Style with_transform(
      AffineTransform value) const {
    Style style = *this;
    style.transform = value;
    return style;
  }
};

[[nodiscard]] inline Style tween(
    const Style& from,
    const Style& to,
    float progress) {
  const float clamped = clamp_animation_progress(progress);
  Style style = clamped < 1.0F ? from : to;
  if (from.background_color.has_value() && to.background_color.has_value()) {
    style.background_color =
        tween(*from.background_color, *to.background_color, clamped);
  }
  if (from.foreground_color.has_value() && to.foreground_color.has_value()) {
    style.foreground_color =
        tween(*from.foreground_color, *to.foreground_color, clamped);
  }
  if (from.border_color.has_value() && to.border_color.has_value()) {
    style.border_color = tween(*from.border_color, *to.border_color, clamped);
  }
  style.opacity = tween_opacity(from.opacity, to.opacity, clamped);
  style.transform = tween(from.transform, to.transform, clamped);
  return style;
}

struct StyleTween {
  Style from;
  Style to;
  AnimationEasing easing = AnimationEasing::linear;

  [[nodiscard]] Style value_at(float progress) const {
    return tween(from, to, ease(easing, progress));
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
  std::optional<int> layer;
  std::optional<float> gap;
  std::optional<AlignItems> align_items;
  std::optional<JustifyContent> justify_content;
  std::optional<float> flex_grow;
  std::optional<float> flex_shrink;
  std::optional<Position> position;
  std::optional<EdgeSizes> inset;
  std::optional<FontDescriptor> font;
  std::optional<float> font_size;
  std::optional<float> opacity;
  std::optional<AffineTransform> transform;

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

  [[nodiscard]] constexpr StyleOverlay with_layer(int value) const {
    StyleOverlay overlay = *this;
    overlay.layer = value;
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

  [[nodiscard]] constexpr StyleOverlay with_flex_grow(float value) const {
    StyleOverlay overlay = *this;
    overlay.flex_grow = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_flex_shrink(float value) const {
    StyleOverlay overlay = *this;
    overlay.flex_shrink = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_position(Position value) const {
    StyleOverlay overlay = *this;
    overlay.position = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_inset(EdgeSizes edges) const {
    StyleOverlay overlay = *this;
    overlay.inset = edges;
    return overlay;
  }

  [[nodiscard]] StyleOverlay with_font(FontDescriptor descriptor) const {
    StyleOverlay overlay = *this;
    overlay.font = descriptor;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_font_size(float value) const {
    StyleOverlay overlay = *this;
    overlay.font_size = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_clip_rect(Rect rect) const {
    StyleOverlay overlay = *this;
    overlay.clip_rect = rect;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_opacity(float value) const {
    StyleOverlay overlay = *this;
    overlay.opacity = value;
    return overlay;
  }

  [[nodiscard]] constexpr StyleOverlay with_transform(
      AffineTransform value) const {
    StyleOverlay overlay = *this;
    overlay.transform = value;
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

[[nodiscard]] inline StyleOverlay style_base_overlay(const Style& style) {
  const Style defaults;
  StyleOverlay overlay;
  overlay.background_color = style.background_color;
  overlay.foreground_color = style.foreground_color;
  overlay.border_color = style.border_color;
  overlay.clip_rect = style.clip_rect;
  if (!same_size(style.preferred_size, defaults.preferred_size)) {
    overlay.preferred_size = style.preferred_size;
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
    if (flags.disabled) {
      style = apply_style_overlay(style, class_style->disabled);
    }
  }

  style = resolved_style(StyleState{
                             .base = style,
                             .hover = local.hover,
                             .focus = local.focus,
                             .disabled = local.disabled,
                         },
                         flags);
  return apply_style_overlay(style, inline_style);
}

} // namespace cgpui
