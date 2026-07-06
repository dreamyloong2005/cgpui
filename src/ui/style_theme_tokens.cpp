#include "style_theme_tokens.hpp"

#include <optional>

namespace {

void apply_color_token(
    std::optional<cgpui::Color>& target,
    const std::optional<cgpui::ThemeTokenId>& token,
    const cgpui::Theme& theme) {
  if (!token.has_value()) {
    return;
  }
  if (std::optional<cgpui::Color> color = theme.color(*token)) {
    target = *color;
  }
}

void apply_spacing_token(
    float& target,
    const std::optional<cgpui::ThemeTokenId>& token,
    const cgpui::Theme& theme) {
  if (!token.has_value()) {
    return;
  }
  if (std::optional<float> spacing = theme.spacing(*token)) {
    target = *spacing;
  }
}

void apply_spacing_token(
    std::optional<float>& target,
    const std::optional<cgpui::ThemeTokenId>& token,
    const cgpui::Theme& theme) {
  if (!token.has_value()) {
    return;
  }
  if (std::optional<float> spacing = theme.spacing(*token)) {
    target = *spacing;
  }
}

void apply_edge_token(
    cgpui::EdgeSizes& target,
    const std::optional<cgpui::ThemeTokenId>& token,
    const cgpui::Theme& theme) {
  if (!token.has_value()) {
    return;
  }
  if (std::optional<float> spacing = theme.spacing(*token)) {
    target = cgpui::edges(*spacing);
  }
}

void apply_edge_token(
    std::optional<cgpui::EdgeSizes>& target,
    const std::optional<cgpui::ThemeTokenId>& token,
    const cgpui::Theme& theme) {
  if (!token.has_value()) {
    return;
  }
  if (std::optional<float> spacing = theme.spacing(*token)) {
    target = cgpui::edges(*spacing);
  }
}

void apply_radius_token(
    cgpui::BorderRadii& target,
    const std::optional<cgpui::ThemeTokenId>& token,
    const cgpui::Theme& theme) {
  if (!token.has_value()) {
    return;
  }
  if (std::optional<float> spacing = theme.spacing(*token)) {
    target = cgpui::BorderRadii::all(*spacing);
  }
}

void apply_radius_token(
    std::optional<cgpui::BorderRadii>& target,
    const std::optional<cgpui::ThemeTokenId>& token,
    const cgpui::Theme& theme) {
  if (!token.has_value()) {
    return;
  }
  if (std::optional<float> spacing = theme.spacing(*token)) {
    target = cgpui::BorderRadii::all(*spacing);
  }
}

} // namespace

namespace cgpui {

Style resolve_style_theme_tokens(Style style, const Theme& theme) {
  apply_color_token(
      style.background_color,
      style.tokens.background_color,
      theme);
  apply_color_token(
      style.foreground_color,
      style.tokens.foreground_color,
      theme);
  apply_color_token(style.border_color, style.tokens.border_color, theme);
  apply_edge_token(style.padding, style.tokens.padding, theme);
  apply_edge_token(style.margin, style.tokens.margin, theme);
  apply_edge_token(style.border_width, style.tokens.border_width, theme);
  apply_radius_token(style.border_radius, style.tokens.border_radius, theme);
  apply_spacing_token(style.gap, style.tokens.gap, theme);
  apply_edge_token(style.inset, style.tokens.inset, theme);
  style.tokens = StyleThemeTokens{};
  return style;
}

StyleOverlay resolve_overlay_theme_tokens(
    StyleOverlay overlay,
    const Theme& theme) {
  apply_color_token(
      overlay.background_color,
      overlay.tokens.background_color,
      theme);
  apply_color_token(
      overlay.foreground_color,
      overlay.tokens.foreground_color,
      theme);
  apply_color_token(overlay.border_color, overlay.tokens.border_color, theme);
  apply_edge_token(overlay.padding, overlay.tokens.padding, theme);
  apply_edge_token(overlay.margin, overlay.tokens.margin, theme);
  apply_edge_token(overlay.border_width, overlay.tokens.border_width, theme);
  apply_radius_token(
      overlay.border_radius,
      overlay.tokens.border_radius,
      theme);
  apply_spacing_token(overlay.gap, overlay.tokens.gap, theme);
  apply_edge_token(overlay.inset, overlay.tokens.inset, theme);
  overlay.tokens = StyleThemeTokens{};
  return overlay;
}

} // namespace cgpui
