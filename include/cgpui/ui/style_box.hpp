#pragma once

#include "cgpui/ui/style_tokens.hpp"
#include "cgpui/ui/style_values.hpp"

#include <limits>
#include <optional>

namespace cgpui {

struct Style {
  std::optional<Color> background_color;
  std::optional<Color> foreground_color;
  std::optional<Color> border_color;
  std::optional<BoxShadow> box_shadow;
  std::optional<Rect> clip_rect;
  FontDescriptor font;
  Size preferred_size;
  Size min_size;
  Size max_size{
      .width = std::numeric_limits<float>::infinity(),
      .height = std::numeric_limits<float>::infinity(),
  };
  PercentageSize percentage_size;
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
  StyleThemeTokens tokens;
  StyleAuthoredTextFields authored_text;

  [[nodiscard]] Style with_background_color(Color color) const;
  [[nodiscard]] Style with_background_color_token(ThemeTokenId id) const;
  [[nodiscard]] Style with_foreground_color(Color color) const;
  [[nodiscard]] Style with_foreground_color_token(ThemeTokenId id) const;
  [[nodiscard]] Style with_preferred_size(Size size) const;
  [[nodiscard]] Style with_min_size(Size size) const;
  [[nodiscard]] Style with_max_size(Size size) const;
  [[nodiscard]] Style with_percentage_size(PercentageSize size) const;
  [[nodiscard]] Style with_width_percent(float percent) const;
  [[nodiscard]] Style with_height_percent(float percent) const;
  [[nodiscard]] Style with_padding(EdgeSizes edges) const;
  [[nodiscard]] Style with_padding_token(ThemeTokenId id) const;
  [[nodiscard]] Style with_margin(EdgeSizes edges) const;
  [[nodiscard]] Style with_margin_token(ThemeTokenId id) const;
  [[nodiscard]] Style with_border_width(EdgeSizes edges) const;
  [[nodiscard]] Style with_border_width_token(ThemeTokenId id) const;
  [[nodiscard]] Style with_border_color(Color color) const;
  [[nodiscard]] Style with_border_color_token(ThemeTokenId id) const;
  [[nodiscard]] Style with_border_radius(BorderRadii radius) const;
  [[nodiscard]] Style with_border_radius_token(ThemeTokenId id) const;
  [[nodiscard]] Style with_box_shadow(BoxShadow shadow) const;
  [[nodiscard]] Style with_overflow(Overflow value) const;
  [[nodiscard]] Style with_z_index(int value) const;
  [[nodiscard]] Style with_layer(int value) const;
  [[nodiscard]] Style with_gap(float value) const;
  [[nodiscard]] Style with_gap_token(ThemeTokenId id) const;
  [[nodiscard]] Style with_align_items(AlignItems value) const;
  [[nodiscard]] Style with_justify_content(JustifyContent value) const;
  [[nodiscard]] Style with_flex_grow(float value) const;
  [[nodiscard]] Style with_flex_shrink(float value) const;
  [[nodiscard]] Style with_position(Position value) const;
  [[nodiscard]] Style with_inset(EdgeSizes edges) const;
  [[nodiscard]] Style with_inset_token(ThemeTokenId id) const;
  [[nodiscard]] Style with_font(FontDescriptor descriptor) const;
  [[nodiscard]] Style with_font_size(float value) const;
  [[nodiscard]] Style with_clip_rect(Rect rect) const;
  [[nodiscard]] Style with_opacity(float value) const;
  [[nodiscard]] Style with_transform(AffineTransform value) const;
};

} // namespace cgpui
