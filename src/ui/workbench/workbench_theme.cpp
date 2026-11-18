#include "cgpui/ui/workbench_theme.hpp"

#include <initializer_list>
#include <string>

namespace cgpui {
namespace {

void apply(const Theme* overrides, const char* name, Color& color) {
  if (overrides == nullptr) return;
  const std::optional<Color> value =
      overrides->color(theme_token(std::string("workbench.") + name));
  if (value.has_value()) color = *value;
}

void light_defaults(WorkbenchTheme& theme) {
  theme.surface = rgb(248, 248, 248);
  theme.surface_alt = rgb(240, 240, 241);
  theme.text = rgb(31, 31, 31);
  theme.text_muted = rgb(97, 97, 97);
  theme.text_disabled = rgb(160, 160, 160);
  theme.border = rgb(222, 222, 224);
  theme.divider = rgb(214, 214, 216);
  theme.hover = rgba(0, 0, 0, 0.06F);
  theme.selection_inactive = rgb(228, 230, 241);
  theme.tab_strip = rgb(236, 236, 237);
  theme.tab_active = rgb(255, 255, 255);
  theme.indent_guide = rgba(0, 0, 0, 0.12F);
  theme.tooltip = rgb(255, 255, 255);
  theme.tooltip_text = rgb(31, 31, 31);
  theme.menu = rgba(252, 252, 252, 0.97F);
  theme.menu_border = rgba(0, 0, 0, 0.12F);
  theme.shadow = rgba(0, 0, 0, 0.22F);
}

void dark_defaults(WorkbenchTheme& theme) {
  theme.surface = rgb(30, 31, 34);
  theme.surface_alt = rgb(24, 25, 28);
  theme.text = rgb(222, 224, 228);
  theme.text_muted = rgb(150, 154, 162);
  theme.text_disabled = rgb(98, 102, 110);
  theme.border = rgb(52, 54, 60);
  theme.divider = rgb(46, 48, 54);
  theme.hover = rgba(255, 255, 255, 0.07F);
  theme.selection_inactive = rgb(55, 58, 66);
  theme.tab_strip = rgb(24, 25, 28);
  theme.tab_active = rgb(30, 31, 34);
  theme.indent_guide = rgba(255, 255, 255, 0.12F);
  theme.tooltip = rgb(44, 46, 52);
  theme.tooltip_text = rgb(226, 228, 232);
  theme.menu = rgba(40, 42, 47, 0.96F);
  theme.menu_border = rgba(255, 255, 255, 0.10F);
  theme.shadow = rgba(0, 0, 0, 0.50F);
}

void high_contrast_defaults(WorkbenchTheme& theme) {
  const Color fore = theme.dark ? rgb(255, 255, 255) : rgb(0, 0, 0);
  const Color back = theme.dark ? rgb(0, 0, 0) : rgb(255, 255, 255);
  for (Color* color : {&theme.surface, &theme.surface_alt, &theme.tab_strip,
                       &theme.tab_active, &theme.tooltip, &theme.menu}) {
    *color = back;
  }
  for (Color* color : {&theme.text, &theme.text_muted, &theme.tooltip_text,
                       &theme.border, &theme.divider, &theme.menu_border}) {
    *color = fore;
  }
  theme.focus_ring_width = 2.0F;
}

} // namespace

WorkbenchTheme workbench_theme(
    const SystemAppearance& appearance,
    const Theme* overrides) {
  WorkbenchTheme theme;
  theme.dark = appearance.mode == SystemAppearanceMode::dark;
  theme.high_contrast = appearance.high_contrast;
  theme.reduced_motion = !appearance.client_area_animation_enabled;
  if (theme.dark) {
    dark_defaults(theme);
  } else {
    light_defaults(theme);
  }
  theme.accent = appearance.accent_color_available ? appearance.accent_color
                                                   : rgb(0, 120, 212);
  theme.accent.a = 1.0F;
  if (theme.high_contrast) high_contrast_defaults(theme);
  theme.accent_text = rgb(255, 255, 255);
  theme.selection = Color{theme.accent.r, theme.accent.g, theme.accent.b,
                          theme.dark ? 0.45F : 0.25F};
  theme.focus_ring = theme.accent;
  theme.dirty = theme.text;
  theme.menu_highlight = theme.accent;
  theme.menu_highlight_text = theme.accent_text;

  const struct {
    const char* name;
    Color* color;
  } fields[]{
      {"surface", &theme.surface}, {"surface_alt", &theme.surface_alt},
      {"text", &theme.text}, {"text_muted", &theme.text_muted},
      {"text_disabled", &theme.text_disabled}, {"accent", &theme.accent},
      {"accent_text", &theme.accent_text}, {"border", &theme.border},
      {"divider", &theme.divider}, {"hover", &theme.hover},
      {"selection", &theme.selection},
      {"selection_inactive", &theme.selection_inactive},
      {"focus_ring", &theme.focus_ring}, {"tab_strip", &theme.tab_strip},
      {"tab_active", &theme.tab_active}, {"dirty", &theme.dirty},
      {"indent_guide", &theme.indent_guide}, {"tooltip", &theme.tooltip},
      {"tooltip_text", &theme.tooltip_text}, {"menu", &theme.menu},
      {"menu_border", &theme.menu_border},
      {"menu_highlight", &theme.menu_highlight},
      {"menu_highlight_text", &theme.menu_highlight_text},
      {"shadow", &theme.shadow},
  };
  for (const auto& field : fields) apply(overrides, field.name, *field.color);
  return theme;
}

} // namespace cgpui
