#include "cgpui/ui/style_cascade.hpp"

#include "style_cascade_overlays.hpp"
#include "style_theme_tokens.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace {

[[nodiscard]] cgpui::Style apply_style_state(
    cgpui::Style style,
    const cgpui::StyleState& state,
    cgpui::StyleStateFlags flags,
    const cgpui::Theme& theme) {
  style = cgpui::apply_style_overlay(
      style,
      cgpui::style_cascade_detail::style_base_overlay(state.base),
      theme);
  if (flags.hovered) {
    style = cgpui::apply_style_overlay(style, state.hover, theme);
  }
  if (flags.focused) {
    style = cgpui::apply_style_overlay(style, state.focus, theme);
  }
  if (flags.active) {
    style = cgpui::apply_style_overlay(style, state.active, theme);
  }
  if (flags.disabled) {
    style = cgpui::apply_style_overlay(style, state.disabled, theme);
  }
  return style;
}

[[nodiscard]] bool is_visiting(
    const std::vector<std::string>& visiting,
    const cgpui::StyleClassId& id) {
  return std::find(visiting.begin(), visiting.end(), id.value) !=
         visiting.end();
}

[[nodiscard]] cgpui::Style apply_class_rule(
    const cgpui::StyleCascade& cascade,
    cgpui::Style style,
    const cgpui::StyleClassId& id,
    cgpui::StyleStateFlags flags,
    const cgpui::Theme& theme,
    std::vector<std::string>& visiting) {
  if (is_visiting(visiting, id)) {
    return style;
  }

  const cgpui::StyleClassRule* rule = cascade.class_rule(id);
  if (rule == nullptr) {
    return style;
  }

  visiting.push_back(id.value);
  for (const cgpui::StyleClassId& reused_id :
       rule->reused_classes.ids()) {
    style = apply_class_rule(
        cascade,
        style,
        reused_id,
        flags,
        theme,
        visiting);
  }
  visiting.pop_back();

  return apply_style_state(style, rule->style, flags, theme);
}

} // namespace

namespace cgpui {

Style apply_style_overlay(
    Style style,
    const StyleOverlay& overlay,
    const Theme& theme) {
  return apply_style_overlay(
      style,
      resolve_overlay_theme_tokens(overlay, theme));
}

Style resolved_style(
    const StyleState& state,
    StyleStateFlags flags,
    const Theme& theme) {
  Style style = resolve_style_theme_tokens(state.base, theme);
  if (flags.hovered) {
    style = apply_style_overlay(style, state.hover, theme);
  }
  if (flags.focused) {
    style = apply_style_overlay(style, state.focus, theme);
  }
  if (flags.active) {
    style = apply_style_overlay(style, state.active, theme);
  }
  if (flags.disabled) {
    style = apply_style_overlay(style, state.disabled, theme);
  }
  return style;
}

Style resolved_style(
    const StyleCascade& cascade,
    const StyleState& local,
    const StyleClasses& classes,
    const StyleOverlay& inline_style,
    StyleStateFlags flags,
    const Theme& theme) {
  Style style = resolve_style_theme_tokens(local.base, theme);
  std::vector<std::string> visiting;
  for (const StyleClassId& class_id : classes.ids()) {
    style =
        apply_class_rule(cascade, style, class_id, flags, theme, visiting);
  }

  if (flags.hovered) {
    style = apply_style_overlay(style, local.hover, theme);
  }
  if (flags.focused) {
    style = apply_style_overlay(style, local.focus, theme);
  }
  if (flags.active) {
    style = apply_style_overlay(style, local.active, theme);
  }
  if (flags.disabled) {
    style = apply_style_overlay(style, local.disabled, theme);
  }
  return apply_style_overlay(style, inline_style, theme);
}

} // namespace cgpui
