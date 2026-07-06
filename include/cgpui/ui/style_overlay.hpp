#pragma once

#include "cgpui/ui/style_tokens.hpp"
#include "cgpui/ui/style_values.hpp"

#include <optional>

namespace cgpui {

struct StyleOverlay {
  std::optional<Color> background_color;
  std::optional<Color> foreground_color;
  std::optional<Color> border_color;
  std::optional<BoxShadow> box_shadow;
  std::optional<Rect> clip_rect;
  std::optional<Size> preferred_size;
  std::optional<Size> min_size;
  std::optional<Size> max_size;
  PercentageSize percentage_size;
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
  StyleThemeTokens tokens;

  [[nodiscard]] StyleOverlay with_background_color(Color color) const;
  [[nodiscard]] StyleOverlay with_background_color_token(
      ThemeTokenId id) const;
  [[nodiscard]] StyleOverlay with_foreground_color(Color color) const;
  [[nodiscard]] StyleOverlay with_foreground_color_token(
      ThemeTokenId id) const;
  [[nodiscard]] StyleOverlay with_preferred_size(Size size) const;
  [[nodiscard]] StyleOverlay with_min_size(Size size) const;
  [[nodiscard]] StyleOverlay with_max_size(Size size) const;
  [[nodiscard]] StyleOverlay with_percentage_size(PercentageSize size) const;
  [[nodiscard]] StyleOverlay with_width_percent(float percent) const;
  [[nodiscard]] StyleOverlay with_height_percent(float percent) const;
  [[nodiscard]] StyleOverlay with_padding(EdgeSizes edges) const;
  [[nodiscard]] StyleOverlay with_padding_token(ThemeTokenId id) const;
  [[nodiscard]] StyleOverlay with_margin(EdgeSizes edges) const;
  [[nodiscard]] StyleOverlay with_margin_token(ThemeTokenId id) const;
  [[nodiscard]] StyleOverlay with_border_width(EdgeSizes edges) const;
  [[nodiscard]] StyleOverlay with_border_width_token(ThemeTokenId id) const;
  [[nodiscard]] StyleOverlay with_border_color(Color color) const;
  [[nodiscard]] StyleOverlay with_border_color_token(ThemeTokenId id) const;
  [[nodiscard]] StyleOverlay with_border_radius(BorderRadii radius) const;
  [[nodiscard]] StyleOverlay with_border_radius_token(ThemeTokenId id) const;
  [[nodiscard]] StyleOverlay with_box_shadow(BoxShadow shadow) const;
  [[nodiscard]] StyleOverlay with_overflow(Overflow value) const;
  [[nodiscard]] StyleOverlay with_z_index(int value) const;
  [[nodiscard]] StyleOverlay with_layer(int value) const;
  [[nodiscard]] StyleOverlay with_gap(float value) const;
  [[nodiscard]] StyleOverlay with_gap_token(ThemeTokenId id) const;
  [[nodiscard]] StyleOverlay with_align_items(AlignItems value) const;
  [[nodiscard]] StyleOverlay with_justify_content(JustifyContent value) const;
  [[nodiscard]] StyleOverlay with_flex_grow(float value) const;
  [[nodiscard]] StyleOverlay with_flex_shrink(float value) const;
  [[nodiscard]] StyleOverlay with_position(Position value) const;
  [[nodiscard]] StyleOverlay with_inset(EdgeSizes edges) const;
  [[nodiscard]] StyleOverlay with_inset_token(ThemeTokenId id) const;
  [[nodiscard]] StyleOverlay with_font(FontDescriptor descriptor) const;
  [[nodiscard]] StyleOverlay with_font_size(float value) const;
  [[nodiscard]] StyleOverlay with_clip_rect(Rect rect) const;
  [[nodiscard]] StyleOverlay with_opacity(float value) const;
  [[nodiscard]] StyleOverlay with_transform(AffineTransform value) const;
};

} // namespace cgpui
