#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/platform_appearance.hpp"
#include "cgpui/ui/style_tokens.hpp"
#include "cgpui/ui/style_values.hpp"

namespace cgpui {

// Colors and metrics shared by the workbench widgets (splitter, tabs, tree
// view, tooltip, popover menu). Defaults follow the system light/dark mode,
// accent and high-contrast setting; any color can be overridden through a
// Theme token named "workbench.<field>" (for example "workbench.accent").
struct WorkbenchTheme {
  bool dark = false;
  bool high_contrast = false;
  bool reduced_motion = false;
  Color surface;
  Color surface_alt;
  Color text;
  Color text_muted;
  Color text_disabled;
  Color accent;
  Color accent_text;
  Color border;
  Color divider;
  Color hover;
  Color selection;
  Color selection_inactive;
  Color focus_ring;
  Color tab_strip;
  Color tab_active;
  Color dirty;
  Color indent_guide;
  Color tooltip;
  Color tooltip_text;
  Color menu;
  Color menu_border;
  Color menu_highlight;
  Color menu_highlight_text;
  Color shadow;
  FontDescriptor font;
  float font_size = 13.0F;
  float focus_ring_width = 1.0F;
  float corner_radius = 6.0F;
};

[[nodiscard]] WorkbenchTheme workbench_theme(
    const SystemAppearance& appearance,
    const Theme* overrides = nullptr);

} // namespace cgpui
